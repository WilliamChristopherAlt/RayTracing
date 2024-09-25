#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math/math_util.h>
#include <OpenGL/shaderClass.h>

struct GlobalUniforms {
	int pad;
    int numTextures;
    unsigned int width;
    unsigned int height;

    int numSpheres;
    int numTriangles;
    int basicShading;
    int basicShadingShadow;

    glm::vec4 basicShadingLightPosition;

    int environmentalLight;
    int maxBounceCount;
    int numRaysPerPixel;
    unsigned int frameIndex;

    glm::vec4 cameraPos;
    glm::vec4 viewportRight;
    glm::vec4 viewportUp;
    glm::vec4 viewportFront;
    glm::vec4 pixelRight;
    glm::vec4 pixelUp;
    glm::vec4 defocusDiskRight;
    glm::vec4 defocusDiskUp; // 24 units
};

class Camera
{
public:
	// Screen
	float scrWidth;
	float scrHeight;
	float aspectRatio;

	// Movement, Rotation
	float mouseSensitivity;

	glm::vec3 position;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 front;
	glm::vec3 worldUp;

	float pitch;
	float yaw;

	float speed;

	// Temporary mouse info to determine correctly rotation
	double lastX;
	double lastY;
	bool firstMouse;

	// Zooming
	float zoomSensitivity;
	float zoom;

	// Viewport vectors
	// WARNING: Back and Front are reveresed terminology for viewport and camera
	float hfov;
	glm::vec3 viewportRight;
	glm::vec3 viewportUp;
	glm::vec3 viewportFront;
	glm::vec3 pixelRight;
	glm::vec3 pixelUp;

	// Affects both viewport and focus effect
	float focusDistance;

	// Defocus
	float defocusAngle;
	float defocusSensitivity;
	glm::vec3 defocusDiskRight;
	glm::vec3 defocusDiskUp;

public:
	enum Movement : uint8_t {
		FORWARD = 0b10000000,
		BACKWARD = 0b01000000,
		LEFT = 0b00100000,
		RIGHT = 0b00010000,
		UP = 0b00001000,
		DOWN = 0b00000100,
		DEFOCUS_UP = 0b00000010,
		DEFOCUS_DOWN = 0b00000001
	};

	Camera(int width, int height, float speed_, const glm::vec3& pos,
		float hfov_, float pitch_, float yaw_,
		float focusDist, float defocusAngle_, float zoom_) :
		scrWidth(width), scrHeight(height), aspectRatio(float(width) / height), speed(speed_),
		position(pos), hfov(hfov_), pitch(pitch_), yaw(yaw_),
		focusDistance(focusDist), defocusAngle(defocusAngle_), zoom(zoom_)
	{
		worldUp = glm::vec3(0, 1, 0);
		updateBasisVectors();

		mouseSensitivity = 1.0f;
		zoomSensitivity = 0.1f;
		defocusSensitivity = 0.1f;

		updateViewportVectors(0.0f);

		lastX = 0.0f;
		lastY = 0.0f;
		firstMouse = true;
	}

	// Movement and Defocus
	void keyboardInput(uint8_t inputBits, float dt)
	{
		if (inputBits & FORWARD)
			position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * speed * dt;
		if (inputBits & BACKWARD)
			position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * speed * dt;
		if (inputBits & LEFT)
			position -= right * speed * dt;
		if (inputBits & RIGHT)
			position += right * speed * dt;
		if (inputBits & UP)
			position += worldUp * speed * dt;
		if (inputBits & DOWN)
			position -= worldUp * speed * dt;
		if (inputBits & DEFOCUS_UP)
		{
			defocusAngle += defocusSensitivity * dt;
			defocusAngle = clamp(defocusAngle, 0.0f, PI / 2.0f);
			updateViewportVectors(0.0f);
		}
		if (inputBits & DEFOCUS_DOWN)
		{
			defocusAngle -= defocusSensitivity * dt;
			defocusAngle = clamp(defocusAngle, 0.0f, PI / 2.0f);
			updateViewportVectors(0.0f);
		}
	}

	// Rotation
	void updateBasisVectors()
	{
		front.x = cos(yaw) * cos(pitch);
		front.y = sin(pitch);
		front.z = sin(yaw) * cos(pitch);
		front = glm::normalize(front);
		right = glm::normalize(glm::cross(worldUp, front));
		up = glm::normalize(glm::cross(front, right));

		updateViewportVectors(0.0f);
	}

	// Zooming or rotation
	void updateViewportVectors(float mouseYOffset)
	{
		zoom += mouseYOffset;

		float h = glm::tan(hfov / 2);
		float viewportWidth = 2 * h / exp(zoom * zoomSensitivity);
		float viewportHeight = viewportWidth / aspectRatio;
		viewportRight = right * viewportWidth * focusDistance;
		viewportUp = up * viewportHeight * focusDistance;
		viewportFront = -front * focusDistance;

		pixelRight = viewportRight / scrWidth;
		pixelUp = viewportUp / scrHeight;

		float defocusRadius = focusDistance * glm::tan(defocusAngle / 2.0f);
		defocusDiskRight = right * defocusRadius;
		defocusDiskUp = up * defocusRadius;
	}

	void updateUniforms(GlobalUniforms& uniforms)
	{
		uniforms.cameraPos = glm::vec4(position, 0.0f);
		uniforms.viewportRight = glm::vec4(viewportRight, 0.0f);
		uniforms.viewportUp = glm::vec4(viewportUp, 0.0f);
		uniforms.viewportFront = glm::vec4(viewportFront, 0.0f);
		uniforms.pixelRight = glm::vec4(pixelRight, 0.0f);
		uniforms.pixelUp = glm::vec4(pixelUp, 0.0f);
		uniforms.defocusDiskRight = glm::vec4(defocusDiskRight, 0.0f);
		uniforms.defocusDiskUp = glm::vec4(defocusDiskUp, 0.0f);
	}

	void mouseCallback(double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = ypos - lastY;

		lastX = xpos;
		lastY = ypos;

		pitch += yoffset / scrWidth * mouseSensitivity / exp(zoom * zoomSensitivity);
		pitch = clamp(pitch, -PI / 2.1f, PI / 2.1f);
		yaw += xoffset / scrWidth * mouseSensitivity / exp(zoom * zoomSensitivity);
		updateBasisVectors();
	}

	void displayInfo()
	{
		std::cout << "Camera terminated at position: (" << position.x << " " << position.y << " " << position.z << ")" << std::endl
			<< "Yaw and pitch: " << yaw << " - " << pitch << std::endl
			<< "Focus distance and defocus angle: " << focusDistance << " - " << defocusAngle << std::endl
			<< "Zoom: " << zoom;
	}

	Camera newCameraWithNewResolution(int w, int h) const
	{
		return Camera(w, h, speed, position, hfov, pitch, yaw, focusDistance, defocusAngle, zoom);
	}
};