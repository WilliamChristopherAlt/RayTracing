#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <OpenGL/shaderClass.h>
#include <OpenGL/VAO.h>
#include <OpenGL/VBO.h>
#include <OpenGL/SSBO.h>
#include <OpenGL/UBO.h>
#include <OpenGL/FBO.h>

#include <RayTracing/Assets/headers/BVH.h>

#include <RayTracing/Assets/headers/camera.h>
#include <RayTracing/Assets/headers/mesh.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <iomanip>
#include <sstream>

const std::string modelFolderName = "toonHouse";
// const std::string modelFolderName = "rinTex";
// const std::string modelFolderName = "mccree";

const int WORK_SIZE_X = 4;
const int WORK_SIZE_Y = 4;
const int WORK_SIZE_Z = 1;

const int SCR_WIDTH = 1000;
const int SCR_HEIGHT = 1000;
// const int SCR_WIDTH = 400;
// const int SCR_HEIGHT = 400;

const int MAX_SPHERES = 5;

const int MAX_BOUNCE_COUNT = 5;
float numRaysPerPixel = 5;
const float RAYS_PER_PIXEL_SENSITIVITY = 10.0f;

const bool BASIC_SHADING = true;
const bool BASIC_SHADING_SHADOW = false;
const glm::vec3 LIGHT_POSITION = glm::vec3(10.0f, 10.0f, 1.0f);
const bool ENVIRONMENTAL_LIGHT = false;

const bool SCREENSHOT_BASIC_SHADING = false;
const int SCREENSHOT_ENVIRONMENTAL_LIGHT = false;
const int SCREENSHOT_MAX_BOUNCE_COUNT = 20;
const int SCREENSHOT_RAYS_PER_PIXEL = 20;
const int SCREENSHOT_FRAMES = 20;

const float CORNELL_LIGHT_BRIGHTNESS = 10.0f;
const float CORNELL_PADDING = 0.25f;
const float CORNELL_LIGHT_SIZE = 0.3f;

const int FPS = 120;
const float SPF = 1.0f / FPS;

// Starting values for camera
float speed = 10.0f;
float hfov = PI / 6;
float pitch = 0.0f;
float yaw = PI / 2.0f;
float focusDistance = 20.0f;
float defocusAngle = 0.0f;
float zoom = 1.0f;
glm::vec3 cameraPos(0.0f, 5.0f, 10.0f);

float vertices[] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f
};

void screenshot(GLFWwindow* window, Camera& camera, VAO& VAO, UBO& UBO, GlobalUniforms& uniforms, Shader& renderShader, ComputeShader& computeShader, Texture2D screenTexture, bool& terminateProgram)
{
	std::cout << "High quality image is being drawn, this may takes a while..." << std::endl;

	uniforms.basicShading = SCREENSHOT_BASIC_SHADING;
	uniforms.environmentalLight = SCREENSHOT_ENVIRONMENTAL_LIGHT;
	uniforms.maxBounceCount = SCREENSHOT_MAX_BOUNCE_COUNT;
	uniforms.numRaysPerPixel = SCREENSHOT_RAYS_PER_PIXEL;
	uniforms.frameIndex = 0;

	camera.updateUniforms(uniforms);
	UBO.Update(&uniforms, sizeof(GlobalUniforms));

	// Allocate memory to store pixel data
	GLubyte* pixels = new GLubyte[3 * SCR_WIDTH * SCR_HEIGHT]{ 0 };
	float* pixelsCumulative = new float[3 * SCR_WIDTH * SCR_HEIGHT]{ 0 };

	// Draw	
	VAO.Bind();

	Texture2D screenshotTexture(SCR_WIDTH, SCR_HEIGHT, nullptr, 0, GL_RGB, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_TEXTURE6);
	FBO FBO(screenshotTexture);

	float renderStart = glfwGetTime();

	for (int i = 0; i < SCREENSHOT_FRAMES; i++)
	{	
		float start = glfwGetTime();
		uniforms.frameIndex = i;
		UBO.Update(&uniforms, sizeof(GlobalUniforms));

		computeShader.Activate();
		glDispatchCompute(ceil(SCR_WIDTH / WORK_SIZE_X), ceil(SCR_HEIGHT / WORK_SIZE_Y), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		screenTexture.SetActive();
		screenTexture.Bind();
		FBO.Bind();
		glClear(GL_COLOR_BUFFER_BIT);
		renderShader.Activate();
		VAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		for (int j = 0; j < 3 * SCR_WIDTH * SCR_HEIGHT; j++)
			pixelsCumulative[j] += pixels[j];
		
		std::cout << "Frame " << i << " done. Render time: " << glfwGetTime() - start << std::endl;
		glfwSwapBuffers(window);
	}

	screenshotTexture.Delete();
	FBO.Delete();

	// Get the average result of all frames
	for (int j = 0; j < 3 * SCR_WIDTH * SCR_HEIGHT; j++)
		pixels[j] = pixelsCumulative[j] / SCREENSHOT_FRAMES;

	// Flip the image vertically(OpenGL's origin is at the bottom-left, but most images start at the top-left)
	for (int y = 0; y < SCR_HEIGHT / 2; y++)
		for (int x = 0; x < SCR_WIDTH * 3; x++)
			std::swap(pixels[y * SCR_WIDTH * 3 + x], pixels[(SCR_HEIGHT - 1 - y) * SCR_WIDTH * 3 + x]);

	std::string path = getPath("\\Images\\test.png", 1);
	stbi_write_png(path.c_str(), SCR_WIDTH, SCR_HEIGHT, 3, pixels, SCR_WIDTH * 3);

	delete[] pixels;
	delete[] pixelsCumulative;

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	float totalRenderTime = glfwGetTime() - renderStart;
	std::cout << "Total render time: " << totalRenderTime / 60.0f << " minutes." << std::endl;
	if (totalRenderTime > 10.0f)
		terminateProgram = true;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	Camera* cameraPtr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	cameraPtr->mouseCallback(xpos, ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera* cameraPtr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	cameraPtr->updateViewportVectors(yoffset);
}

void keyBoardInput(GLFWwindow* window, Camera& camera, float dt, bool& isScreenshot)
{
	uint8_t inputBits = 0;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, true);
		return;
	}
	if (glfwGetKey(window, GLFW_KEY_W) && !glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::FORWARD;
	if (glfwGetKey(window, GLFW_KEY_S) && !glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::BACKWARD;
	if (glfwGetKey(window, GLFW_KEY_A))
		inputBits |= Camera::LEFT;
	if (glfwGetKey(window, GLFW_KEY_D))
		inputBits |= Camera::RIGHT;
	if (glfwGetKey(window, GLFW_KEY_W) && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::UP;
	if (glfwGetKey(window, GLFW_KEY_S) && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::DOWN;
	if (glfwGetKey(window, GLFW_KEY_Z) && !glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::DEFOCUS_UP;
	if (glfwGetKey(window, GLFW_KEY_Z) && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		inputBits |= Camera::DEFOCUS_DOWN;
	if (glfwGetKey(window, GLFW_KEY_X) && !glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
	{
		numRaysPerPixel += RAYS_PER_PIXEL_SENSITIVITY * dt;
		numRaysPerPixel = clamp(numRaysPerPixel, 1.1f, 200.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_X) && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
	{
		numRaysPerPixel -= RAYS_PER_PIXEL_SENSITIVITY * dt;
		numRaysPerPixel = clamp(numRaysPerPixel, 1.1f, 200.0f);
	}
	
	camera.keyboardInput(inputBits, dt);
	isScreenshot = (glfwGetKey(window, GLFW_KEY_S) && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL));
}

void addSkyLightPlane(std::vector<RTXTriangle>& rtxTriangles, std::vector<BVHTriangle>& bvhTriangles, int lightMtlIndex)
{
	BoundingBox sceneBounds;
	for (const BVHTriangle& tri : bvhTriangles)
		sceneBounds.growToInclude(tri);

	glm::vec3 sceneSize = sceneBounds.size();
	float minX = sceneBounds.min.x;
	float maxX = sceneBounds.max.x;
	float minY = sceneBounds.min.y;
	float maxY = sceneBounds.max.y;
	float minZ = sceneBounds.min.z;
	float maxZ = sceneBounds.max.z;
	
	float planeY = maxY + sceneSize.y * 0.1f;

	glm::vec3 boxSize = glm::vec3(maxX, maxY, maxZ) - glm::vec3(minX, minY, minZ);

	std::vector<glm::vec3> skyCorners = 
	{
		glm::vec3(minX, planeY, maxZ),
		glm::vec3(maxX, planeY, maxZ),
		glm::vec3(minX, planeY, minZ),
		glm::vec3(maxX, planeY, minZ),
	};

	int skyCornersIndicies[2][3] =
	{
		{0, 3, 1},
		{0, 2, 3}
	};

	std::vector<RTXTriangle> skyTrianglesRTX;
	for (int i = 0 ; i < 2; i++)
		skyTrianglesRTX.push_back(RTXTriangle(lightMtlIndex, glm::vec4(skyCorners[skyCornersIndicies[i][0]], 0.0f),  glm::vec4(skyCorners[skyCornersIndicies[i][1]], 0.0f),
							 glm::vec4(skyCorners[skyCornersIndicies[i][2]], 0.0f), glm::vec2(), glm::vec2(), glm::vec2()));
	std::vector<BVHTriangle> skyTrianglesBVH;
	for (int i = 0 ; i < 2; i++)
		skyTrianglesBVH.push_back(BVHTriangle(skyCorners[skyCornersIndicies[i][0]], skyCorners[skyCornersIndicies[i][1]], skyCorners[skyCornersIndicies[i][2]]));

	rtxTriangles.insert(rtxTriangles.end(), skyTrianglesRTX.begin(), skyTrianglesRTX.end());
	bvhTriangles.insert(bvhTriangles.end(), skyTrianglesBVH.begin(), skyTrianglesBVH.end());
	rtxTriangles.insert(rtxTriangles.end(), skyTrianglesRTX.begin(), skyTrianglesRTX.end());
	bvhTriangles.insert(bvhTriangles.end(), skyTrianglesBVH.begin(), skyTrianglesBVH.end());
}

void addCornellBox(std::vector<RTXTriangle>& rtxTriangles, std::vector<BVHTriangle>& bvhTriangles, float lightSize, float padPercentage, int lightMtlIndex)
{
	BoundingBox sceneBounds;
	for (const BVHTriangle& tri : bvhTriangles)
		sceneBounds.growToInclude(tri);

	glm::vec3 sceneSize = sceneBounds.size();
	float minX = sceneBounds.min.x - sceneSize.x * padPercentage;
	float maxX = sceneBounds.max.x + sceneSize.x * padPercentage;
	float minY = sceneBounds.min.y - sceneSize.y * padPercentage * 0.1f;
	float maxY = sceneBounds.max.y + sceneSize.y * padPercentage;
	float minZ = sceneBounds.min.z - sceneSize.z * padPercentage;
	float maxZ = sceneBounds.max.z + sceneSize.z * padPercentage;

	std::vector<glm::vec3> corners = 
	{
		glm::vec3(minX, minY, maxZ),
		glm::vec3(maxX, minY, maxZ),
		glm::vec3(minX, maxY, maxZ),
		glm::vec3(maxX, maxY, maxZ),
		glm::vec3(minX, minY, minZ),
		glm::vec3(maxX, minY, minZ),
		glm::vec3(minX, maxY, minZ),
		glm::vec3(maxX, maxY, minZ),
	};

	glm::vec3 boxSize = glm::vec3(maxX, maxY, maxZ) - glm::vec3(minX, minY, minZ);
	float centerX = (maxX + minX) / 2.0f;
	float centerZ = (maxZ + minZ) / 2.0f;
	float lightMinX = centerX - lightSize * boxSize.x / 2.0f;
	float lightMaxX = centerX + lightSize * boxSize.x / 2.0f;
	float lightMinZ = centerZ - lightSize * boxSize.z / 2.0f;
	float lightMaxZ = centerZ + lightSize * boxSize.z / 2.0f;
	float lightY = maxY - 1e-3f;

	std::vector<glm::vec3> lightCorners =
	{
		glm::vec3(lightMinX, lightY, lightMaxZ),
		glm::vec3(lightMaxX, lightY, lightMaxZ),
		glm::vec3(lightMinX, lightY, lightMinZ),
		glm::vec3(lightMaxX, lightY, lightMinZ),
	};

	int triCornersIndicies[12][3] =
	{
		{0, 3, 1},
		{0, 2, 3},
		{0, 5, 4},
		{0, 1, 5},
		{0, 6, 2},
		{0, 4, 6},
		{7, 1, 3},
		{7, 5, 1},
		{7, 2, 6},
		{7, 3, 2},
		{7, 4, 5},
		{7, 6, 4}
	};

	int lightCornersIndicies[4][3] =
	{
		{0, 3, 1},
		{0, 2, 3},
		{0, 1, 3},
		{0, 3, 2}
	};

	std::vector<RTXTriangle> cornellCornersRTX;
	for (int i = 0 ; i < 12; i++)
	{
		Material mat = Material();
		cornellCornersRTX.push_back(RTXTriangle(0, glm::vec4(corners[triCornersIndicies[i][0]], 0.0f),  glm::vec4(corners[triCornersIndicies[i][1]], 0.0f),
							 glm::vec4(corners[triCornersIndicies[i][2]], 0.0f), glm::vec2(), glm::vec2(), glm::vec2()));
	}
	std::vector<BVHTriangle> cornellCornersBVH;
	for (int i = 0 ; i < 12; i++)
		cornellCornersBVH.push_back(BVHTriangle(corners[triCornersIndicies[i][0]], corners[triCornersIndicies[i][1]], corners[triCornersIndicies[i][2]]));

	std::vector<RTXTriangle> cornellLightRTX;
	for (int i = 0 ; i < 4; i++)
		cornellLightRTX.push_back(RTXTriangle(lightMtlIndex, glm::vec4(lightCorners[lightCornersIndicies[i][0]], 0.0f),  glm::vec4(lightCorners[lightCornersIndicies[i][1]], 0.0f),
							 glm::vec4(lightCorners[lightCornersIndicies[i][2]], 0.0f), glm::vec2(), glm::vec2(), glm::vec2()));
	std::vector<BVHTriangle> cornellLightBVH;
	for (int i = 0 ; i < 4; i++)
		cornellLightBVH.push_back(BVHTriangle(lightCorners[triCornersIndicies[i][0]], lightCorners[triCornersIndicies[i][1]], lightCorners[triCornersIndicies[i][2]]));

	rtxTriangles.insert(rtxTriangles.end(), cornellCornersRTX.begin(), cornellCornersRTX.end());
	bvhTriangles.insert(bvhTriangles.end(), cornellCornersBVH.begin(), cornellCornersBVH.end());
	rtxTriangles.insert(rtxTriangles.end(), cornellLightRTX.begin(), cornellLightRTX.end());
	bvhTriangles.insert(bvhTriangles.end(), cornellLightBVH.begin(), cornellLightBVH.end());
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLfloat ndcVertices[] =
{
	// Top left
	-1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	// Bottom right
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

int main(int argc, char* argv[])
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(0);
	

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Loading mesh data
	std::vector<RTXTriangle> rtxTriangles;
	std::vector<BVHTriangle> bvhTriangles;
	std::vector<Material> materials;
	std::vector<Texture2D> textures;
	getTrianglesData_("Data\\" + modelFolderName, 1, rtxTriangles, bvhTriangles, materials, textures);

	Material mat;
	mat.makeLight(glm::vec3(1.0f), CORNELL_LIGHT_BRIGHTNESS);
	materials.push_back(mat);
	addCornellBox(rtxTriangles, bvhTriangles, CORNELL_LIGHT_SIZE, CORNELL_PADDING, materials.size() - 1);
	// addSkyLightPlane(rtxTriangles, bvhTriangles, materials.size() - 1);

	BVH BVH(bvhTriangles, rtxTriangles);

	// for (RTXTriangle& tri : rtxTriangles)
	// 	tri.material.makeSpecular(glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f), 1.0f, 1.0f);

	// build and compile shaders
	// -------------------------
	std::string shaderFolderPath = getPath("Assets\\Shaders", 1);
	Shader renderShader(shaderFolderPath + "\\vert.glsl", shaderFolderPath + "\\newFrag.glsl");
	ComputeShader computeShader(shaderFolderPath + "\\compute.glsl");
	renderShader.Activate();
	renderShader.setInt("tex", 5);

	int textureUnits[MAX_TEXTURES] = {0, 1, 2, 3, 4};
	renderShader.setIntArray("textures", textureUnits, sizeof(textureUnits));

	// Texture for the compute shader to draw on
	Texture2D screenTexture(SCR_WIDTH, SCR_HEIGHT, GL_TEXTURE5);
	for (int i = 0; i < textures.size(); i++)
	{
		computeShader.setInt(("texture" + std::to_string(i)).c_str(), i);
		textures[i].SetActive();
		textures[i].Bind();
	}

	// SSBOs for triangles and nodes
	SSBO trianglesSSBO(rtxTriangles.data(), sizeof(RTXTriangle) * rtxTriangles.size(), 1);
	SSBO nodesSSBO(BVH.allNodes.data(), sizeof(Node) * BVH.allNodes.size(), 2);
	SSBO materialsSSBO(materials.data(), sizeof(Material) * materials.size(), 3);

	// Set shader's constants
	computeShader.bindSSBOToBlock(trianglesSSBO, "TrianglesBlock");
	computeShader.bindSSBOToBlock(nodesSSBO, "NodesBlock");
	computeShader.bindSSBOToBlock(materialsSSBO, "MaterialsBlock");

	// Transfer uniforms with UBO
	GlobalUniforms uniforms;
	UBO UBO(3, sizeof(GlobalUniforms));

	// Create camera
	Camera camera = Camera(SCR_WIDTH, SCR_HEIGHT, speed, cameraPos, hfov, pitch, yaw, focusDistance, defocusAngle, zoom);

	// Is later used by glfwGetWindowUserPointer in glfwSetCursorPosCallback and glfwSetScrollCallback to get the camera, avoiding global variables
	glfwSetWindowUserPointer(window, &camera);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// VAO, VBO
	VAO VAO;
	VAO.Bind();
	VBOSimple VBO(ndcVertices, sizeof(ndcVertices));
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);
	VAO.Unbind();
	VBO.Unbind();

	// render loop
	// -----------
	int frameIndex = 0;
	float lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		// Get deltaTime
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		if (deltaTime < SPF)
			continue;
		lastFrame = currentFrame;

		// Set window title with FPS
		std::ostringstream fps;
		fps << std::fixed << std::setprecision(2) << 1.0f / deltaTime;
		std::string title = "Demo - FPS:" + fps.str();
		glfwSetWindowTitle(window, title.c_str());

		// Keyboard input
		bool isScreenshot;
		keyBoardInput(window, camera, deltaTime, isScreenshot);

		// Screenshot
		bool terminateProgram = false;
		if (isScreenshot)
			screenshot(window, camera, VAO, UBO, uniforms, renderShader, computeShader, screenTexture, terminateProgram);

		if (terminateProgram)
			glfwSetWindowShouldClose(window, true);		

		// Uniforms
		uniforms.numTextures = textures.size();
		uniforms.width = SCR_WIDTH;
		uniforms.height = SCR_HEIGHT;
		uniforms.numSpheres = 0;
		uniforms.numTriangles = rtxTriangles.size();
		uniforms.basicShading = BASIC_SHADING;
		uniforms.basicShadingShadow = BASIC_SHADING_SHADOW;
		uniforms.basicShadingLightPosition = glm::vec4(LIGHT_POSITION, 0.0f);
		uniforms.environmentalLight = ENVIRONMENTAL_LIGHT;
		uniforms.maxBounceCount = MAX_BOUNCE_COUNT;
		uniforms.numRaysPerPixel = numRaysPerPixel;
		uniforms.frameIndex = frameIndex;
		frameIndex++;
		// Update uniforms based on changes of position, rotation, and zooming
		camera.updateUniforms(uniforms);

		UBO.Update(&uniforms, sizeof(GlobalUniforms));

		computeShader.Activate();
		glDispatchCompute(ceil(SCR_WIDTH / WORK_SIZE_X), ceil(SCR_HEIGHT / WORK_SIZE_Y), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// render image to quad
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderShader.Activate();
		VAO.Bind();

		screenTexture.SetActive();
		screenTexture.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	screenTexture.Delete();
	computeShader.Delete();
	renderShader.Delete();
	VAO.Delete();
	VBO.Delete();
	UBO.Delete();
	trianglesSSBO.Delete();
	nodesSSBO.Delete();
	materialsSSBO.Delete();

	for (Texture2D& tex : textures)
		tex.Delete();

	glfwTerminate();

	return EXIT_SUCCESS;
}