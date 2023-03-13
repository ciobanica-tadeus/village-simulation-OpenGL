#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, -5.0f, 3.0f),
    glm::vec3(10.0f, 4.0f, 10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.2f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scene;
gps::Model3D wheel;
gps::Model3D door;
gps::Model3D maner;
gps::Model3D car;
gps::Model3D wheelFrontLeft;
gps::Model3D wheelBackLeft;
gps::Model3D wheelFrontRight;
gps::Model3D wheelBackRight;
gps::Model3D teaspot;
gps::Model3D drop1;
gps::Model3D drop2;
gps::Model3D drop3;
gps::Model3D lego;

GLfloat angle;
GLfloat door_angle;
GLfloat maner_angle;
//fog
GLuint isFog = 0;
GLuint isFogLoc;
    
// shaders
gps::Shader myBasicShader;
gps::Shader reflection;

std::vector<const GLchar*> faces;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

float delta = 0.0f;
float movementSpeed = 0.15f;
float carMovement = 0.0f;
double lastTimeStamp = glfwGetTime();
double pitch1 = 0.0f;
double yaw1 = 0.0f;
double pitch = 0.0f;
double yaw = 0.0f;
bool canOpenDoor = false;
bool scenePresentation = false, rain = false;
int counter = 0;
//raindrops
float x = 3.0f;
float x1 = 15.0f;
float x2 = 26.0f;


//----------Miscare moara--------------
void updateDelta(double elapsedSeconds) {
    delta += movementSpeed * elapsedSeconds;

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
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //double pitch, yaw;

    //get yaw and pitch angle of the current window
    glfwGetCursorPos(myWindow.getWindow(), &yaw, &pitch);

    //rotate camera 
    myCamera.rotate(-pitch / 4.0f, yaw / 4.0f);

    ////show message in console
    //printf("pitch: %f \n yaw: %f \n\n", pitch, yaw);

    //view = myCamera.getViewMatrix();
    //myBasicShader.useShaderProgram();
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //// compute normal matrix for scene
    //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

}

void processMovement() {

    //the interior of the polygon is filled.
    //------------SOLID MODE--------------------
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_1)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    //boundary edges of the polygon are drawn as line segments
    //-------------- WIREFRAME MODE------------------
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_2)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //polygon vertices that are marked as the start of a boundary edge are drawn as points
    //--------------- POLYGON MODE--------------------
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_3)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    //----------------SMOOTH MODE--------------------
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_4)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

    }

    //-------MOVE FORWARD-----------
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

    //-------MOVE BACKWARD-----------
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }
    //-------MOVE LEFT-----------
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

    //-------MOVE RIGHT-----------
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

    //--------ROTATE LEFT------------
    if (pressedKeys[GLFW_KEY_LEFT]) {
        yaw1 -= 0.7f;
        myCamera.rotate(-(pitch1 + pitch) / 4.0f, (yaw1 + yaw) / 4.0f);
    }

    //--------ROTATE RIGHT----------
    if (pressedKeys[GLFW_KEY_RIGHT]) {
        yaw1 += 0.7f;
        myCamera.rotate(- (pitch1 + pitch)/4.0f , (yaw1 + yaw) / 4.0f );
    }

    //--------ROTATE TEAPOT LEFT------
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 0.05f;
        // update model matrix for scene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    //--------ROTATE TEAPOT RIGHT-------
    if (pressedKeys[GLFW_KEY_E]) {
        angle += 0.05f;
        // update model matrix for scene
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scene
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    //-------MOVE UP-----------
    if (pressedKeys[GLFW_KEY_UP]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    //-------MOVE DOWN-----------
    if (pressedKeys[GLFW_KEY_DOWN]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }

    ///-----------OPEN DOOR---------------------
    if (pressedKeys[GLFW_KEY_O]) {
        if (door_angle < 90.f && canOpenDoor) {
            door_angle += 1.0f;
        }  
        else {
            if (canOpenDoor) {
                door_angle = 90.0f;
            }
            
        }
        /*door_angle += 0.1f;
        printf("%f ,", door_angle);*/
    }

    ///-----------CLOSE DOOR--------------------
    if (pressedKeys[GLFW_KEY_P]) {
        if (door_angle > 0.0f && canOpenDoor) door_angle -= 1.0f;
        else {
            if (canOpenDoor)
            door_angle = 0.0f;
        }
    }

    ///-----------OPEN MANER---------------------
    if (pressedKeys[GLFW_KEY_K]) {
        if (maner_angle < 50.f) {
            maner_angle += 1.0f;
        }
        else {
            maner_angle = 50.0f;
            canOpenDoor = true;
        }
        /*door_angle += 0.1f;
        printf("%f ,", door_angle);*/
    }

    ///-----------CLOSE MANER--------------------
    if (pressedKeys[GLFW_KEY_L]) {
        if (maner_angle > 0.0f) maner_angle -= 1.0f;
        else {
            maner_angle = 0.0f;
            canOpenDoor = false;
        }
    }

    //---------- CAR IS MOVING FORWARD---------------
    if (pressedKeys[GLFW_KEY_F]) {
        carMovement += 0.2f;
    }

    //---------- CAR IS MOVING BACKWARD----------------
    if (pressedKeys[GLFW_KEY_G]) {
        carMovement -= 0.2f;
    }

    //---------- ACTIVATE FOG-------------------------
    if (pressedKeys[GLFW_KEY_5])
    {
        isFog = 1;
        myBasicShader.useShaderProgram();
        glUniform1i(isFogLoc, isFog);
    }

    //---------- DESACTIVATE FOG ----------------------
    if (pressedKeys[GLFW_KEY_6])
    {
        isFog = 0;
        myBasicShader.useShaderProgram();
        glUniform1i(isFogLoc, isFog);
    }

    //----------ACTIVATE RAIN--------------
    if (pressedKeys[GLFW_KEY_7]) {
        rain = true;
    }

    //----------DESACTIVATE RAIN------------
    if (pressedKeys[GLFW_KEY_8]) {
        rain = false;
    }

    //--------ACTIVATE PRESENTATION MODE--------------
    if (pressedKeys[GLFW_KEY_9]) {
        scenePresentation = true;
        myCamera = gps::Camera(
            glm::vec3(0.0f, -5.0f, 3.0f),
            glm::vec3(10.0f, -4.5f, 10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    }

    //----------DESACTIVATE PRESENTATION MODE------------
    if (pressedKeys[GLFW_KEY_0]) {

        pressedKeys[GLFW_KEY_W] = false;
        pressedKeys[GLFW_KEY_RIGHT] = false;
        pressedKeys[GLFW_KEY_O] = false;
        pressedKeys[GLFW_KEY_P] = false;
        pressedKeys[GLFW_KEY_K] = false;
        pressedKeys[GLFW_KEY_L] = false;
        pressedKeys[GLFW_KEY_F] = false;
        pressedKeys[GLFW_KEY_G] = false;
        scenePresentation = false;
        counter = 0;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDisable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teaspot.LoadModel("models/teapot/teapot.obj");
    scene.LoadModel("models/scene/myModel.obj");
    wheel.LoadModel("models/wheel/wheel.obj");
    door.LoadModel("models/wc_door/wc_door.obj");
    maner.LoadModel("models/wc_maner/wc_maner.obj");
    car.LoadModel("models/dacia/dacia.obj");
    wheelFrontLeft.LoadModel("models/carWheel/car_wheel.obj");
    wheelBackLeft.LoadModel("models/carWheel/car_wheel.obj");
    wheelFrontRight.LoadModel("models/carWheel/car_wheel.obj");
    wheelBackRight.LoadModel("models/carWheel/car_wheel.obj");
    drop1.LoadModel("models/picatura/picatura3.obj");
    drop2.LoadModel("models/picatura/picatura3.obj");
    drop3.LoadModel("models/picatura/picatura3.obj");
    lego.LoadModel("models/lego_figure/lego.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert","shaders/basic.frag");
    //reflection.loadShader("shaders/reflection.vert", "shaders/reflection.frag");
    /*faces.push_back("models/skybox/right.tga");
    faces.push_back("models/skybox/left.tga");
    faces.push_back("models/skybox/top.tga");
    faces.push_back("models/skybox/bottom.tga");
    faces.push_back("models/skybox/back.tga");
    faces.push_back("models/skybox/front.tga");*/

    faces.push_back("models/greenhaze/greenhaze_ft.tga");  
    faces.push_back("models/greenhaze/greenhaze_bk.tga");
  
    faces.push_back("models/greenhaze/greenhaze_up.tga");        
    faces.push_back("models/greenhaze/greenhaze_dn.tga");
    faces.push_back("models/greenhaze/greenhaze_rt.tga");
    faces.push_back("models/greenhaze/greenhaze_lf.tga");

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    mySkyBox.Load(faces);
}

void initUniforms() {
    
	myBasicShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 10000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    //-------- isFog -----------------
    myBasicShader.useShaderProgram();
    isFogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isFog");
    glUniform1i(isFogLoc, isFog);
    

  /*  myBasicShader.useShaderProgram();
    reflectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "reflectionE");
    glUniform1i(reflectionLoc, reflectionE);*/

    /////-----------REFLECTION----------------

    //reflection.useShaderProgram();

    //model = glm::mat4(1.0f);
    //modelLoc = glGetUniformLocation(reflection.shaderProgram, "model");
    ////glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //// get view matrix for current camera
    //view = myCamera.getViewMatrix();
    //viewLoc = glGetUniformLocation(reflection.shaderProgram, "view");
    //// send view matrix to shader
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //// compute normal matrix for teapot
    //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //normalMatrixLoc = glGetUniformLocation(reflection.shaderProgram, "normalMatrix");

    //// create projection matrix
    //projection = glm::perspective(glm::radians(45.0f),
    //    (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
    //    0.1f, 10000.0f);
    //projectionLoc = glGetUniformLocation(reflection.shaderProgram, "projection");
    //// send projection matrix to shader
    //glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    ////set the light direction (direction towards the light)
    //lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    //lightDirLoc = glGetUniformLocation(reflection.shaderProgram, "lightDir");
    //// send light dir to shader
    //glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    ////set light color
    //lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    //lightColorLoc = glGetUniformLocation(reflection.shaderProgram, "lightColor");
    //// send light color to shader
    //glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //isFogLoc = glGetUniformLocation(reflection.shaderProgram, "isFog");
    //glUniform1i(isFogLoc, isFog);
    
}

void renderTeaPot(gps::Shader shader) {

    // select active shader program
    shader.useShaderProgram();


    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, -7.65f, 101.9f)) *
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    teaspot.Draw(shader);
}

void renderModel(gps::Shader shader) {

    // select active shader program
    shader.useShaderProgram();
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    scene.Draw(shader);
}

void renderSkyBox(gps::Shader shader) {

    shader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    mySkyBox.Draw(shader, view, projection);
}

void renderWheel(gps::Shader shader) {
    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(-8.5f, 16.0f, 122.0f)) *
        glm::rotate(glm::mat4(1.0f), delta, glm::vec3(0.0f, 0.0f, -1.0f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    wheel.Draw(shader);
}

void renderDoor(gps::Shader shader) {

    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(30.74f, -8.0f, 107.4f)) *
        glm::rotate(model, glm::radians(door_angle), glm::vec3(0, 1, 0));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    door.Draw(shader);
}

void renderManer(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(32.1f, -7.0f, 107.35f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(3.14f), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(-maner_angle), glm::vec3(0, 0, 1));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    maner.Draw(shader);
}

void renderCar(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, carMovement)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, -8.6f, -1.0f));
        

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    car.Draw(shader);
}

void renderWheelFrontLeft(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, carMovement)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-6.3f, -8.35f, 0.9f)) *
        glm::rotate(glm::mat4(1.0f), carMovement, glm::vec3(1.0f, 0.0f, 0.0f));
        
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    wheelFrontLeft.Draw(shader);
}

void renderWheelBackLeft(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, carMovement)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-6.3f, -8.5f, -2.41f)) *
        glm::rotate(glm::mat4(1.0f), carMovement, glm::vec3(1.0f, 0.0f, 0.0f));
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    wheelBackLeft.Draw(shader);
}

void renderWheelFrontRight(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, carMovement)) *
        glm::translate(glm::mat4(1.0f), glm::vec3(-8.15f, -8.35f, 0.9f)) *
        glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), carMovement, glm::vec3(1.0f, 0.0f, 0.0f));
        
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    wheelFrontRight.Draw(shader);
}

void renderWheelBackRight(gps::Shader shader) {
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, carMovement))*
        glm::translate(glm::mat4(1.0f), glm::vec3(-8.15f, -8.5f, -2.435f)) *
        glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), carMovement, glm::vec3(1.0f, 0.0f, 0.0f));
        
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    wheelBackRight.Draw(shader);
}

void renderDrops(gps::Shader shader) {
    //for raindrops falling
    x = x - 0.5f;
    if (x < -24.0f) x = 10.0f;

    x1 = x1 - 0.5f;
    if (x1 < -24.0f ) x1 = 10.0f;

    x2 = x2 - 0.5f;
    if (x2 < -24.0f ) x2 = 10.0f;


    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model =
        glm::translate(model, glm::vec3(-55.0f, 0.0f, 57.0f)) *
        glm::rotate(glm::mat4(1.0f), 3.14f / -2.0f , glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(model, glm::vec3(0.0f, x, 0.0f));
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    drop1.Draw(shader);

    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model =
        glm::translate(model, glm::vec3(-55.0f, 0.0f, 57.0f)) *
        glm::rotate(glm::mat4(1.0f), 3.14f / -2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(model, glm::vec3(0.0f, x1, 0.0f));
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    drop2.Draw(shader);

    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model =
        glm::translate(model, glm::vec3(-55.0f,0.0f,57.0f)) * 
        glm::rotate(glm::mat4(1.0f), 3.14f / -2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(model, glm::vec3(0.0f, x2, 0.0f));
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    drop3.Draw(shader);
}

void renderLego(gps::Shader shader) {

    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, -8.0f, -94.0f));

    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    lego.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //update view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    //reflection.useShaderProgram();
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //// compute normal matrix for scene
    //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //normalMatrixLoc = glGetUniformLocation(reflection.shaderProgram, "normalMatrix");
    //render teapot
    renderTeaPot(myBasicShader);

	//render the scene
	renderModel(myBasicShader);

    //wheel
    renderWheel(myBasicShader);

    //door
    renderDoor(myBasicShader);

    //maner 
    renderManer(myBasicShader);

    //car
    renderCar(myBasicShader);

    //left front wheel
    renderWheelFrontLeft(myBasicShader);

    //left back wheel
    renderWheelBackLeft(myBasicShader);

    //right front wheel
    renderWheelFrontRight(myBasicShader);

    //right back wheel
    renderWheelBackRight(myBasicShader);
    
    //renderDrops
    if (rain) {
        renderDrops(myBasicShader);
    }
    //reflectionE = 1;
    //reflectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "reflectionE");
    //glUniform1i(reflectionLoc,reflectionE);

    ////render statue
    renderLego(myBasicShader);
    ///reflectionE = 0;
    //glUniform1i(reflectionLoc, reflectionE);
    //scene presentation
    if (scenePresentation) {

        if (counter <= 50 ) {
            pressedKeys[GLFW_KEY_W] = true;
            counter++;
        }
        if (counter > 50 && counter <= 125) {
            pressedKeys[GLFW_KEY_W] = false;
            pressedKeys[GLFW_KEY_RIGHT] = true;
            counter++;
        }
        if (counter > 125 && counter <= 250) {
            pressedKeys[GLFW_KEY_RIGHT] = false;
            pressedKeys[GLFW_KEY_W] = true;
            counter++;
        }
        if (counter > 250 && counter <= 252) {
            pressedKeys[GLFW_KEY_W] = false;
            pressedKeys[GLFW_KEY_5] = true;
            counter++;
        }
        if (counter > 252 && counter <= 380) {
            pressedKeys[GLFW_KEY_5] = false;
            pressedKeys[GLFW_KEY_RIGHT] = true;
            counter++;
        }
        
        if (counter > 380 && counter <= 382) {
            pressedKeys[GLFW_KEY_RIGHT] = false;
            pressedKeys[GLFW_KEY_6] = true;
            counter++;
        }
        if (counter > 382 && counter <= 450) {
            pressedKeys[GLFW_KEY_6] = false;
            pressedKeys[GLFW_KEY_K] = true;
            counter++;
        }

        if (counter > 450 && counter <= 510) {
            pressedKeys[GLFW_KEY_K] = false;
            pressedKeys[GLFW_KEY_O] = true;
            counter++;
        }
        if (counter > 510 && counter <= 560) {
            pressedKeys[GLFW_KEY_O] = false;
            pressedKeys[GLFW_KEY_P] = true;
            counter++;
        }
        if (counter > 560 && counter <= 562) {
            pressedKeys[GLFW_KEY_P] = false;
            pressedKeys[GLFW_KEY_7] = true;
            counter++;
        }
        if (counter > 562 && counter <= 1500) {
            pressedKeys[GLFW_KEY_7] = false;
            pressedKeys[GLFW_KEY_RIGHT] = true;
            counter++;
        }      
        if (counter > 1500 && counter <= 1700) {
            pressedKeys[GLFW_KEY_RIGHT] = false;
            pressedKeys[GLFW_KEY_F] = true;
            counter++;
        }

        if (counter > 1700 && counter <= 2000) {
            pressedKeys[GLFW_KEY_F] = false;
            pressedKeys[GLFW_KEY_G] = true;
            counter++;
        }

        if (counter > 2000 && counter <= 2100) {
            pressedKeys[GLFW_KEY_G] = false;
            pressedKeys[GLFW_KEY_UP] = true;
            counter++;
        }

        if (counter > 2100 && counter <= 2500) {
            pressedKeys[GLFW_KEY_UP] = false;
            pressedKeys[GLFW_KEY_W] = true;
            counter++;
        }

        if (counter > 2500 && counter <= 3000) {
            pressedKeys[GLFW_KEY_W] = false;
            pressedKeys[GLFW_KEY_RIGHT] = true;
            counter++;
        }
        if (counter > 3000) {
            pressedKeys[GLFW_KEY_RIGHT] = false;
            scenePresentation = false;
            counter++;
        }
    }
    
    // skyBox
    renderSkyBox(skyboxShader);
   
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
