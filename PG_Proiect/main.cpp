#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

std::ofstream fout("movements1.txt");
std::ifstream fin("movements.txt");

// window
gps::Window myWindow;
int retina_width, retina_height;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::mat4 modelSpaceship;

std::vector<glm::mat4> modelAlienMatrices;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::mat4 lightRotation;

glm::vec3 lightLaser;
glm::vec3 lightLaser2;
std::vector<glm::vec3> lightLaserColors;
const int numLaserColors = 3;
int indexColor;
bool divColorChange{ false };

const int numSpotLights = 5;
std::vector<glm::vec3> neonLights;
std::vector<glm::vec3> neonColors;

// shadow
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

GLuint shadowMapFBO;
GLuint depthMapTexture;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lightLaserDirLoc;
GLuint lightLaserDirLoc2;
GLuint lightLaserColorLoc;
GLuint lightningLoc;

// skybox
gps::SkyBox mySkyBox;
std::vector<const GLchar*> skyboxFaces;

// camera
gps::Camera myCamera(
    glm::vec3(76.8245f, 10.3486f, -57.0691f),
    glm::vec3(76.974f, 10.3234f, -58.0576f),
    glm::vec3(0.0f, 1.0f, 0.0f));
gps::Camera animationCamera(
    glm::vec3(76.8245f, 10.3486f, -57.0691f),
    glm::vec3(76.974f, 10.3234f, -58.0576f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.9f;
const float cameraYaw{ 5.0f };
const float cameraPitch{ 5.0f };

const float near{ 0.1f };
const float far{ 1000.0f };

GLboolean pressedKeys[1024];

// models
gps::Model3D ground;
gps::Model3D spaceship;
gps::Model3D cargoship;
gps::Model3D tube;

gps::Model3D lightCube;
gps::Model3D laser;
gps::Model3D laser2;
gps::Model3D dome;
gps::Model3D satellites;
std::vector<gps::Model3D*> lightnings{ new gps::Model3D(), new gps::Model3D() };

gps::Model3D screenQuad;

GLfloat angle;
GLfloat lightAngle;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;
gps::Shader screenQuadShader;
gps::Shader lightCubeShader;

const auto numAliens = 3;
struct alienModelType {
    gps::Model3D* head;
    gps::Model3D* body;

    glm::vec3 origin;
};
std::vector<alienModelType> aliens(numAliens);

// mouse control
const float sensitivity{ 0.09f };
bool firstMouse{ true };
double lastX{ 512 };
double lastY{ 384 };
double xOffset;
double yOffset;

float fov{ 45.0f };

bool showDepthMap{ false };
int motionAnimationOn{ false };
std::vector<int> motionHeadAnimationOn{ false, false, false };
std::vector<float> headRotationAngle{-1.0f, 1.0f, 1.0f};

int lightningEffectOn{ false };
int laserAnimationOn{ false };
bool presentationAnimationOn{ false };

float animationFactor{ 0.01f };
float distanceForward{ 10.0f };
float lightningIntensity{ 1.0f };
int frame = 0.0f;

auto headAngle{ 0.0f };

float laserTimeAnimation;
glm::vec3 currPoint(-47.0372f, 26.7004f, 73.0815f);
glm::vec3 currPoint2(-48.1364f, 27.299f, 72.2083f);
glm::vec3 finalPoint(-14.7674f, 11.4022f, 31.7048f);

bool printAction{ false };

glm::mat4 computeLightSpaceTrMatrix() {
    glm::vec4 lightPos = lightRotation * glm::vec4(lightDir, 0.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 75.0f;
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        presentationAnimationOn = !presentationAnimationOn;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        printAction = !printAction;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        myCamera.printCoordinates();
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (presentationAnimationOn) {
        return;
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        showDepthMap = !showDepthMap;
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        motionAnimationOn = !motionAnimationOn;
    }

    if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
        lightningEffectOn = !lightningEffectOn;
    }

    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        laserAnimationOn = !laserAnimationOn;
    }

    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        motionHeadAnimationOn[0] = !motionHeadAnimationOn[0];
    }

    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        motionHeadAnimationOn[1] = !motionHeadAnimationOn[1];
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        motionHeadAnimationOn[2] = !motionHeadAnimationOn[2];
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {   
    if (presentationAnimationOn) {
        return;
    }

    fov -= yoffset;
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    else if (fov > 45.0f) {
        fov = 45.0f;
    }

    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(fov),
        static_cast<float>(myWindow.getWindowDimensions().width) / static_cast<float>(myWindow.getWindowDimensions().height),
        near, far);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (presentationAnimationOn) {
        return;
    }

    if (printAction) {
        fout << 9 << ' ' << xpos << ' ' << ypos << ' ';
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    xOffset = xpos - lastX;
    yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xOffset *= sensitivity;
    yOffset *= sensitivity;
    myCamera.rotate(yOffset, xOffset);
}

void processVisualizationMode() {
    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
        myBasicShader.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        lightRotation = glm::rotate(model, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
        myBasicShader.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        lightRotation = glm::rotate(model, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
    }

    if (pressedKeys[GLFW_KEY_UP]) {
        if (printAction) {
            fout << 7 << ' ';
        }
        myCamera.rotate(cameraPitch, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        if (printAction) {
            fout << 8 << ' ';
        }
        myCamera.rotate(-cameraPitch, 0);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    if (pressedKeys[GLFW_KEY_LEFT]) {
        if (printAction) {
            fout << 6 << ' ';
        }
        myCamera.rotate(0, -cameraYaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    if (pressedKeys[GLFW_KEY_RIGHT]) {
        if (printAction) {
            fout << 5 << ' ';
        }   
        myCamera.rotate(0, cameraYaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

	if (pressedKeys[GLFW_KEY_W]) {
        if (printAction) {
            fout << 1 << ' ';
        }
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
        if (printAction) {
            fout << 3 << ' ';
        }
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
        if (printAction) {
            fout << 2 << ' ';
        }
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
        if (printAction) {
            fout << 4 << ' ';
        }
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "Spaceships");

    //for RETINA display
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scrollCallback);
}

void initOpenGLState() {
    glClearColor(0.3, 0.3, 0.3, 0.1);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initSkyboxFaces() {
    skyboxFaces.push_back("models/skybox/right.png");
    skyboxFaces.push_back("models/skybox/left.png");
    skyboxFaces.push_back("models/skybox/top.png");
    skyboxFaces.push_back("models/skybox/bottom.png");
    skyboxFaces.push_back("models/skybox/back.png");
    skyboxFaces.push_back("models/skybox/front.png");
}

void initModels() {
    ground.LoadModel("models/ground/ground.obj");
    spaceship.LoadModel("models/spaceship/spaceship.obj");
    cargoship.LoadModel("models/cargoship/cargoship.obj");
    tube.LoadModel("models/tube/tube.obj");
    dome.LoadModel("models/dome/dome.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    laser.LoadModel("models/laser/laser.obj");
    laser2.LoadModel("models/laser/laser2.obj");
    satellites.LoadModel("models/satellite/satellites.obj");
    lightnings[0]->LoadModel("models/lightning/lightning.obj");
    lightnings[1]->LoadModel("models/lightning/lightning2.obj");
    mySkyBox.Load(skyboxFaces);

    modelAlienMatrices.push_back(glm::mat4(1.0f));
    modelAlienMatrices.push_back(glm::mat4(1.0f));
    modelAlienMatrices.push_back(glm::mat4(1.0f));

    for (auto i = 0; i < numAliens; i++) {
        aliens[i].head = new gps::Model3D();
        aliens[i].body = new gps::Model3D();
    }

    aliens[0].head->LoadModel("models/aliens/alienHead1.obj");
    aliens[0].body->LoadModel("models/aliens/alienBody1.obj");
    aliens[0].origin = glm::vec3(68.7558f, 13.9893f, 32.9394f);


    aliens[1].head->LoadModel("models/aliens/alienHead2.obj");
    aliens[1].body->LoadModel("models/aliens/alienBody2.obj");
    aliens[1].origin = glm::vec3(-19.8711f, 21.3075f, 16.2738f);

    aliens[2].head->LoadModel("models/aliens/alienHead3.obj");
    aliens[2].body->LoadModel("models/aliens/alienBody3.obj");
    aliens[2].origin = glm::vec3(45.8619f, 16.276f, -21.9501f);
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
    depthMapShader.loadShader(
        "shaders/depthMap.vert",
        "shaders/depthMap.frag");
    screenQuadShader.loadShader(
        "shaders/screenQuad.vert", 
        "shaders/screenQuad.frag");
    lightCubeShader.loadShader(
        "shaders/lightCube.vert", 
        "shaders/lightCube.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
    if (presentationAnimationOn) {
        view = animationCamera.getViewMatrix();
    }
    else {
        view = myCamera.getViewMatrix();
    }
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
    projection = glm::perspective(glm::radians(fov),
        static_cast<float>(myWindow.getWindowDimensions().width) /
        static_cast<float>(myWindow.getWindowDimensions().height),
        near, far);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-7.26524f, 60.5659f, 15.2692f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    lightLaser = glm::vec3(-47.0372f, 26.7004f, 73.0815f);
    lightLaser2 = glm::vec3(-48.1364f, 27.299f, 72.2083f);
    lightLaserDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightLaserDir");
    glUniform3fv(lightLaserDirLoc, 1, glm::value_ptr(lightLaser));
    lightLaserDirLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "lightLaserDir2");
    glUniform3fv(lightLaserDirLoc2, 1, glm::value_ptr(lightLaser2));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //set light color
    lightLaserColors.push_back(glm::vec3(1.0f, 0.0f, 1.0f)); 
    lightLaserColors.push_back(glm::vec3(0.0f, 1.0f, 1.0f)); 
    lightLaserColors.push_back(glm::vec3(1.0f, 1.0f, 0.0f)); 
    indexColor = rand() % numLaserColors;

    lightLaserColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightLaserColor");
    // send light color to shader
    glUniform3fv(lightLaserColorLoc, 1, glm::value_ptr(lightLaserColors[indexColor]));

    neonLights.push_back(glm::vec3(70.329f, 21.631f, -24.1364f));
    neonLights.push_back(glm::vec3(72.4841f, 20.8997f, -36.6096f));
    neonLights.push_back(glm::vec3(74.6938f, 20.0156f, -49.0611f));
    neonLights.push_back(glm::vec3(76.8277f, 19.1655f, -61.0206f));
    neonLights.push_back(glm::vec3(79.0456f, 18.2584f, -73.5539f));

    neonColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    neonColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    neonColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    neonColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    neonColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonPos[0]"), 1, glm::value_ptr(neonLights[0]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonPos[1]"), 1, glm::value_ptr(neonLights[1]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonPos[2]"), 1, glm::value_ptr(neonLights[2]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonPos[3]"), 1, glm::value_ptr(neonLights[3]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonPos[4]"), 1, glm::value_ptr(neonLights[4]));
    
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonColor[0]"), 1, glm::value_ptr(neonColors[0]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonColor[1]"), 1, glm::value_ptr(neonColors[1]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonColor[2]"), 1, glm::value_ptr(neonColors[2]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonColor[3]"), 1, glm::value_ptr(neonColors[3]));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "neonColor[4]"), 1, glm::value_ptr(neonColors[4]));

    // lightning uniform
    lightningLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightIntensity");

    // light cube
    lightCubeShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(
        lightCubeShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(
        lightCubeShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(
        lightCubeShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // skybox
    skyboxShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(
        skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(
        skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void initDepthFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);

    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderCargoSpaceship(gps::Shader shader, const bool depthPass) {
    shader.useShaderProgram();
    
    modelSpaceship = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, animationFactor));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelSpaceship));

    if (motionAnimationOn) {
        animationFactor += 0.1f;
    }

    // todo: implememnt the curve

    //send teapot normal matrix data to shader
    if (!depthPass) {
        glm::mat3 normalMat = glm::mat3(glm::inverseTranspose(view * modelSpaceship));
        glUniformMatrix3fv(glGetUniformLocation(
            shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));
    }

    cargoship.Draw(shader);
}

void renderAliens(gps::Shader shader, const bool depthPass) {
    for (auto i = 0; i < numAliens; i++) {
        shader.useShaderProgram();

        // draw body
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        //send teapot normal matrix data to shader
        if (!depthPass) {
            glUniformMatrix3fv(glGetUniformLocation(
                shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }
        aliens[i].body->Draw(shader);
        
        // implement rotation of the head1,
        if (motionHeadAnimationOn[i]) {
            modelAlienMatrices[i] = glm::translate(modelAlienMatrices[i], aliens[i].origin);
            modelAlienMatrices[i] = glm::rotate(modelAlienMatrices[i], glm::radians(headRotationAngle[i]), glm::vec3(0.0f, 1.0f, 0.0f));
            modelAlienMatrices[i] = glm::translate(modelAlienMatrices[i], -aliens[i].origin);
        }

        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelAlienMatrices[i]));
        //send teapot normal matrix data to shader
        if (!depthPass) {
            glm::mat3 normalMat = glm::mat3(glm::inverseTranspose(view * modelAlienMatrices[i]));
            glUniformMatrix3fv(glGetUniformLocation(
                shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));
        }
        aliens[i].head->Draw(shader);
    }
}

void renderLightCube() {
    lightCubeShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(lightCubeShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

    glm::mat4 modelLightCube = lightRotation;
    modelLightCube = glm::translate(modelLightCube, 1.0f * lightDir);
    modelLightCube = glm::scale(modelLightCube, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLightCube));

    lightCube.Draw(lightCubeShader);
}

bool laserRandered{ true };

void renderLaser() {
    lightCubeShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform3fv(glGetUniformLocation(lightCubeShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightLaserColors[indexColor]));

    if (laserRandered) {
        glm::mat4 modelLaser = glm::translate(glm::mat4(1.0f), -currPoint);
        modelLaser = glm::translate(modelLaser, lightLaser);
        glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLaser));
        laser.Draw(lightCubeShader);
    }
    else {
        glm::mat4 modelLaser = glm::translate(glm::mat4(1.0f), -currPoint2);
        modelLaser = glm::translate(modelLaser, lightLaser2);
        glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLaser));
        laser2.Draw(lightCubeShader);
    } 
}

void renderSpots() {
    myBasicShader.useShaderProgram();
    for (auto i = 0; i < numSpotLights; i++) {
        std::string str = "spotIntensity[" + std::to_string(i) + "]";
        float r = static_cast<float> (rand()) / static_cast <float> (RAND_MAX);

        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, str.c_str()), r);
    }
}

void renderPointLight() {
    myBasicShader.useShaderProgram();

    if (laserRandered) {
        lightLaser = laserTimeAnimation * finalPoint + (1 - laserTimeAnimation) * currPoint;
        if (laserTimeAnimation <= 1.0f) {
            laserTimeAnimation += 0.09f;
        }
        else {
            laserTimeAnimation = 0.0f;
            laserRandered = !laserRandered;
        }
    }
    else {
        lightLaser2 = laserTimeAnimation * finalPoint + (1 - laserTimeAnimation) * currPoint2;
        if (laserTimeAnimation <= 1.0f) {
            laserTimeAnimation += 0.09f;
        }
        else {
            laserTimeAnimation = 0.0f;
            laserRandered = !laserRandered;
        }
    }

    

    if (divColorChange) {
        indexColor = rand() % numLaserColors;
        glUniform3fv(lightLaserColorLoc, 1, glm::value_ptr(lightLaserColors[indexColor]));
    } 
    divColorChange = !divColorChange;

    glUniform3fv(lightLaserDirLoc, 1, glm::value_ptr(lightLaser));
}

void renderLightning() {
    myBasicShader.useShaderProgram();
    if (lightningEffectOn) {
        switch (frame)
        {
        case 0:
            glUniform1f(lightningLoc, 80.0f);
            lightnings[0]->Draw(myBasicShader);
            break;
        case 2:
            glUniform1f(lightningLoc, 10.0f);
            lightnings[0]->Draw(myBasicShader);
            break;
        case 6:
            glUniform1f(lightningLoc, 100.0f);
            lightnings[0]->Draw(myBasicShader);
            lightnings[1]->Draw(myBasicShader);
            break;

        case 7: {
            glUniform1f(lightningLoc, 1.0f);
            frame = 0;
            lightningEffectOn = !lightningEffectOn;
        } break;
        
        default:
            break;
        }
        frame += 1;
    }
}

void renderObj(gps::Shader shader, bool depthPass) {
    if (laserAnimationOn) {
        renderPointLight();
    }
    renderSpots();

    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    if (!depthPass) {
        glUniformMatrix3fv(glGetUniformLocation(
            shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw
    tube.Draw(shader);
    spaceship.Draw(shader);
    ground.Draw(shader);
    dome.Draw(shader);
    satellites.Draw(shader);

    renderLightCube();
    if (laserAnimationOn) {
        renderLaser();
    }

    renderAliens(shader, depthPass);
    renderCargoSpaceship(shader, depthPass);
}

void renderTheScene() {
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    if (presentationAnimationOn) {
        view = animationCamera.getViewMatrix();
    }
    else {
        view = myCamera.getViewMatrix();
    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightRotation = glm::rotate(model, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    renderObj(myBasicShader, false);

    renderLightning();
}

void createDepthMap() {
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderObj(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderDepthMap(GLuint texture) {
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT);

    screenQuadShader.useShaderProgram();

    //bind the depth map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

    glDisable(GL_DEPTH_TEST);
    screenQuad.Draw(screenQuadShader);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


float animationAngleInc = 3.0f;
float animationSpeed = 1.0f;

const glm::vec3 animationStartPoint(76.8245f, 10.3486f, -57.0691f); // first camera position for the animation
const glm::vec3 animationStartTarget(76.974f, 10.3234f, -58.0576f); // first camera target

static void sendViewMatrix() {
    //update view matrix
    view = animationCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

double animLastX;
double animLastY;
bool animFirstMouse{ true };
double animXOffset;
double animYOffset;

enum struct SimulationSteps : uint8_t {
    Init,
    Wait,
    Done,
};

std::vector<SimulationSteps> stayForVec{
    SimulationSteps::Init,
    SimulationSteps::Init,
    SimulationSteps::Init,
    SimulationSteps::Init,
    SimulationSteps::Init
};

struct AnimationParamsType {
    std::pair<glm::vec3, glm::vec3> controlPoint;
    SimulationSteps& stayFor;
    int& effectOn;
    float pauseTime;
};

const auto numOfPoints = 5;
const std::vector<AnimationParamsType> animationParams{
    {{{70.4431f, 12.4025f, -6.45043f}, {70.3559f, 12.4025f, -5.45424f}}, stayForVec[0], motionHeadAnimationOn[0], 2.0f},
    {{{70.3646f, 12.4025f, -5.55386f}, {69.4583f, 12.4025f, -5.97647f}}, stayForVec[1], motionHeadAnimationOn[2], 2.0f},
    {{{31.0855f, 16.9864f, 67.4611f},  {30.572f, 17.0726f, 66.6073f}}, stayForVec[2], motionAnimationOn, 4.0f},
    {{{-56.7333f, 37.5374f, 30.818f},  {-56.2069f, 37.1011f, 31.5478f}}, stayForVec[3], laserAnimationOn, 4.0f},
    {{{-39.3758f, 28.4652f, 35.4617f}, {-38.8731f, 27.9672f, 34.7551f}}, stayForVec[4], lightningEffectOn, 4.0f}
};

time_t start, end;

static bool handleStopAnimMovement(const int index) {
    time(&end);

    const auto camPos = animationCamera.getCamPos();
    const auto camTar = animationCamera.getTargetPos();
    if (fabs(camPos.x - animationParams[index].controlPoint.first.x) <= 1e-4 &&
        fabs(camPos.y - animationParams[index].controlPoint.first.y) <= 1e-4 &&
        fabs(camPos.z - animationParams[index].controlPoint.first.z) <= 1e-4 &&
        fabs(camTar.x - animationParams[index].controlPoint.second.x) <= 1e-4 &&
        fabs(camTar.y - animationParams[index].controlPoint.second.y) <= 1e-4 &&
        fabs(camTar.z - animationParams[index].controlPoint.second.z) <= 1e-4) {

        if (animationParams[index].stayFor == SimulationSteps::Init) {
            animationParams[index].stayFor = SimulationSteps::Wait;
            animationParams[index].effectOn = true;
            time(&start);
        }
    }

    if (animationParams[index].stayFor == SimulationSteps::Wait && 
        fabs(static_cast<float>(end - start) - animationParams[index].pauseTime) >= 1e-5) {
        return false;
    }
    else if (animationParams[index].stayFor == SimulationSteps::Wait) {
        animationParams[index].effectOn = false;
        animationParams[index].stayFor = SimulationSteps::Done;
    }
    return true;
}

void presentationAnimation() {
    if (!presentationAnimationOn) {
        return;
    }

    for (auto i = 0; i < numOfPoints; i++) {
        if (!handleStopAnimMovement(i)) {
            return;
        }
    }

    int move;
    fin >> move;

    switch (move)
    {
    case 1: {
        // move forward
        animationCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        sendViewMatrix();
    } break;
    case 2: {
        animationCamera.move(gps::MOVE_LEFT, cameraSpeed);
        sendViewMatrix();
    } break;
    case 3: {
        animationCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        sendViewMatrix();
    } break;
    case 4: {
        animationCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        sendViewMatrix();
    } break;
    case 5: {
        animationCamera.rotate(0, cameraYaw);
        sendViewMatrix();
    } break;
    case 6: {
        animationCamera.rotate(0, -cameraYaw);
        sendViewMatrix();
    } break;
    case 7: {
        animationCamera.rotate(cameraPitch, 0);
        sendViewMatrix();
    } break;
    case 8: {
        animationCamera.rotate(-cameraPitch, 0);
        sendViewMatrix();
    } break;

    case 9: {
        double x, y;
        fin >> x >> y;

        if (animFirstMouse) {
            animLastX = x;
            animLastY = y;
            animFirstMouse = false;
        }

        animXOffset = x - animLastX;
        animYOffset = animLastY - y;
        animLastX = x;
        animLastY = y;

        animXOffset *= sensitivity;
        animYOffset *= sensitivity;
        animationCamera.rotate(animYOffset, animXOffset);
    } break;

    default:
        break;
    }
}

void renderScene() {
    createDepthMap();

    if (showDepthMap) {
        renderDepthMap(depthMapTexture);
    }
    else {
        renderTheScene();
        presentationAnimation();

    }

    mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    
    //cleanup code for your own data
    for (auto i = 0; i < numAliens; i++) {
        delete aliens[i].head;
        delete aliens[i].body;
    }
    delete lightnings[0];
    delete lightnings[1];

    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    initOpenGLState();
    initSkyboxFaces();
	initModels();
	initShaders();
	initUniforms();
    
    initDepthFBO();
    setWindowCallbacks();

	glCheckError();

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processVisualizationMode();
        processMovement();

        renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
