#include "glad.h"
#include "shader.h"
#include "model.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <experimental/filesystem>
using namespace glm;

void printHelp();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, double xpos, double ypos);
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);

void createFloor(Shader shader);
void createFrontWall(Shader shader,float trX, float trY, float trZ, float scX, float scY, float scZ, float r, float g, float b);
void createSideWall(Shader shader,float trX, float trY, float trZ, float scX, float scY, float scZ, float r, float g, float b);
void drawChair(Shader shader, float x, float y, float z);
void drawTable(Shader shader, float x, float y, float z);
void drawComputerTable(Shader shader, float x, float y, float z);
void drawCube(Shader shader, float trX, float trY, float trZ, float scX, float scY, float scZ,float r, float g, float b);

//Windows Dimension
const GLint WIDTH = 1366, HEIGHT = 768;

GLuint VAO, VBO, EBO;

//camera values
glm::vec3 gscale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 cPosInit = glm::vec3(0.0f, 2.0f, 4.0f);
glm::vec3 cFrontInit = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cUpInit = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraPos = cPosInit;
glm::vec3 cameraFront = cFrontInit;
glm::vec3 cameraUp = cUpInit;
float lastX = WIDTH/2.0;
float lastY = HEIGHT/2.0;
bool firstFrame = true;
bool cctvMode = false;
float yaw = -90.0f;
float pitch = 0.0f;
float fovy = 45.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
   
   //Initailize GLFW
	if (!glfwInit()) {
		printf("GLFW initialization failed. Exiting..\n");
		glfwTerminate();
		return 1;
	}

	//Setup GLFW Windows Properties
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//No backward compatibilty
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Forward Compatibilty
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//Create a window
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "The Classroom", NULL, NULL);
	if (!mainWindow) {
		printf("Window Not Created. Exiting..\n");
		glfwTerminate();
		return 1;
	}

	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(mainWindow, mouseCallBack);
	glfwSetScrollCallback(mainWindow, scrollCallBack);

	//Get Buffer Size Info
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

    //load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, bufferWidth, bufferHeight);

    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
	Shader ourShader("shaders/vertexshader.glsl", "shaders/fragmentshader.glsl");

    // load models
    //Model ground("obj/ground.obj");
    Model tDesk("obj/tDesk.obj");
    Model cupboard("obj/cupboard.obj");
    Model blackboard("obj/blackboard.obj");
    Model clock("obj/clock.obj");
    Model cctv("obj/camera.obj");
    Model computer("obj/computer.obj");
    Model bookshelf("obj/bookshelf.obj");

    printHelp();

    //Loop until window closed
    while (!glfwWindowShouldClose(mainWindow))
    {
        float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		ourShader.setVec3("gscale", gscale);
		ourShader.setBool("cctv", cctvMode);

		//Get and Handle User Events 
		glfwPollEvents();

        glClearColor(0.41f, 0.41f, 0.41f, 0.9f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        //construct and send projection matrix to shader
        glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(fovy), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        //construct and send view matrix to shader
        glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        createFloor(ourShader);
        createFrontWall(ourShader,0.0f,0.3f,-2.24f,1.875f,0.8f,1.0f,0.69f,0.69f,0.0f);
        createSideWall(ourShader,1.875f,0.3f,0.0f,2.24f,0.8f,1.0f,0.6f,0.6f,0.6f);
        createSideWall(ourShader,-1.875f,0.3f,0.0f,2.24f,0.8f,1.0f,0.6f,0.6f,0.6f);

        //draw chairs and tables
		for (float i = -1.1; i < 1.1; i += 0.65) 
        {
			for (float j = -1.0; j < 2.0; j += 0.8)
			{
				drawChair(ourShader, i+0.05, -0.44, j+0.2);
				drawTable(ourShader, i+0.05, -0.44, (j-0.1)+0.2);
			}
		}
		drawComputerTable(ourShader, -1.75f, -0.44f, 1.8f);


        // render the loaded models
        mat4 groundMat;
        groundMat = translate(groundMat, vec3(0.0f, -0.5f, 0.0f));

        mat4 tDeskMat;
        tDeskMat = translate(groundMat, vec3(-0.035f, 0.0f, -1.8f));
        tDeskMat = rotate(tDeskMat, radians(0.0f), vec3(0.0, 1.0, 0.0));
        tDeskMat = scale(tDeskMat, vec3(0.08f, 0.08f, 0.08f));
        ourShader.setMat4("model", tDeskMat);
        tDesk.Draw(ourShader);

        mat4 cupboardMat;
        cupboardMat = translate(groundMat, vec3(1.6f, 0.0f, 1.8f));
        cupboardMat = rotate(cupboardMat, radians(-90.0f), vec3(0.0, 1.0, 0.0));
        cupboardMat = scale(cupboardMat, vec3(0.07f, 0.07f, 0.07f));
        ourShader.setMat4("model", cupboardMat);
        cupboard.Draw(ourShader);

        mat4 blackboardMat;
        blackboardMat = translate(groundMat, vec3(-0.035f, 0.35f, -2.1f));
        blackboardMat = rotate(blackboardMat, radians(180.0f), vec3(0.0, 1.0, 0.0));
        blackboardMat = scale(blackboardMat, vec3(0.02f, 0.02f, 0.02f));
        ourShader.setMat4("model", blackboardMat);
        blackboard.Draw(ourShader);

        mat4 clockMat;
        clockMat = translate(groundMat, vec3(-1.84f, 0.8f, 0.0f));
        clockMat = rotate(clockMat, radians(-90.0f), vec3(0.0, 1.0, 0.0));
        clockMat = scale(clockMat, vec3(0.0055f, 0.0055f, 0.0055f));
        ourShader.setMat4("model", clockMat);
        clock.Draw(ourShader);

        mat4 cctvMat;
        cctvMat = translate(groundMat, vec3(1.76f, 1.3f, 0.0f));
        cctvMat = rotate(cctvMat, radians(180.0f), vec3(0.0, 1.0, 0.0));
        cctvMat = scale(cctvMat, vec3(0.003f, 0.003f, 0.003f));
        ourShader.setMat4("model", cctvMat);
        cctv.Draw(ourShader);

        mat4 computerMat;
        computerMat = translate(groundMat, vec3(-1.7f, 0.185f, 1.8f));
        computerMat = rotate(computerMat, radians(-90.0f), vec3(0.0, 1.0, 0.0));
        computerMat = scale(computerMat, vec3(0.04f, 0.04f, 0.04f));
        ourShader.setMat4("model", computerMat);
        computer.Draw(ourShader);

        mat4 bookshelfMat;
        bookshelfMat = translate(groundMat, vec3(-1.72f, 0.0f, -1.5f));
        bookshelfMat = rotate(bookshelfMat, radians(0.0f), vec3(0.0, 1.0, 0.0));
        bookshelfMat = scale(bookshelfMat, vec3(0.055f, 0.055f, 0.055f));
        ourShader.setMat4("model", bookshelfMat);
        bookshelf.Draw(ourShader);

        glfwSwapBuffers(mainWindow);
    }

    //terminate glfw
    glfwTerminate();
    return 0;
}

void createFloor(Shader shader)
{

	GLfloat vertices[] = {
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mat4 model(1.0f);
    model = translate(model, vec3(0.0f, -0.5f, 0.0f));
    model = rotate(model, radians(90.0f), vec3(1.0, 0.0, 0.0));
    model = scale(model, vec3(2.5f*0.75f, 2.8f*0.8f, 2.8f*0.9f));
    shader.setMat4("model", model);

	shader.setVec3("color", vec3(0.8f,0.8f,0.8f));
    glBindVertexArray(VAO);
    GLuint isize = sizeof(indices)/sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void createFrontWall(Shader shader,float trX, float trY, float trZ, float scX, float scY, float scZ, float r, float g, float b){

	GLfloat vertices[] = {
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mat4 model(1.0f);
    model = translate(model, vec3(trX, trY, trZ));
    model = rotate(model, radians(0.0f), vec3(1.0, 0.0, 0.0));
    model = scale(model, vec3(scX, scY, scZ));
    shader.setMat4("model", model);

	shader.setVec3("color", vec3(r,g,b));
    glBindVertexArray(VAO);
    GLuint isize = sizeof(indices)/sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void createSideWall(Shader shader,float trX, float trY, float trZ, float scX, float scY, float scZ, float r, float g, float b){

	GLfloat vertices[] = {
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mat4 model(1.0f);
    model = translate(model, vec3(trX, trY, trZ));
    model = rotate(model, radians(90.0f), vec3(0.0, 1.0, 0.0));
    model = scale(model, vec3(scX, scY, scZ));
    shader.setMat4("model", model);

	shader.setVec3("color", vec3(r,g,b));
    glBindVertexArray(VAO);
    GLuint isize = sizeof(indices)/sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void drawChair(Shader shader, float x, float y, float z) {
	// legs of chair
	drawCube(shader, x,y,z, 0.5,3.0,0.5, 0.5,0.2,0.2);
	drawCube(shader, x+0.1,y,z, 0.5,3.0,0.5, 0.5,0.2,0.2);
	drawCube(shader, x+0.1,y,z + 0.1, 0.5,3.0,0.5, 0.5,0.2,0.2);
	drawCube(shader, x, y, z + 0.1, 0.5, 3.0, 0.5, 0.5,0.2,0.2);

	// seat of chair
	drawCube(shader, x+0.045,y+0.06,z+0.045, 3.0,1.0,3.0, 0.5,0.2,0.2);
	
	// backrest of chair
	drawCube(shader, x+0.045,y+0.12,z+0.091, 3.0,4.0,1.0, 0.5,0.2,0.2);

}

void drawTable(Shader shader, float x, float y, float z)
{
	// legs of table
	drawCube(shader, x-0.05,y,z-0.15, 0.5,6.0,0.5, 0.0,0.5,0.0);
	drawCube(shader, x+0.15,y,z-0.15, 0.5,6.0,0.5, 0.0,0.5,0.0);
	drawCube(shader, x+0.15,y,z, 0.5,6.0,0.5, 0.0,0.5,0.0);
	drawCube(shader, x-0.05,y,z, 0.5,6.0,0.5, 0.0,0.5,0.0);

	//table top
	drawCube(shader, x+0.05,y+0.12,z-0.07,7.0,0.3,5.0, 0.0,0.5,0.0);
}

void drawComputerTable(Shader shader, float x, float y, float z)
{
	// legs of table
	int sc = 1.5;
	drawCube(shader, x-0.05,y,z-0.15, 0.5*sc , 6.0*sc, 0.5*sc, 0.0,0.5,0.0);
	drawCube(shader, x+0.05,y,z-0.15, 0.5*sc , 6.0*sc, 0.5*sc, 0.0,0.5,0.0);
	drawCube(shader, x+0.05,y,z+0.15, 0.5*sc , 6.0*sc, 0.5*sc, 0.0,0.5,0.0);
	drawCube(shader, x-0.05,y,z+0.15, 0.5*sc , 6.0*sc, 0.5*sc, 0.0,0.5,0.0);

	//table top
	drawCube(shader, x,y+0.12,z,5.5*sc,0.3*sc,9.0*sc, 0.0,0.5,0.0);
}

void drawCube(Shader shader, float trX, float trY, float trZ, float scX, float scY, float scZ,float r, float g, float b)
{

	GLfloat vertices[] = {
		-0.02f, -0.02f, -0.02f,
		0.02f, -0.02f, -0.02f,
		0.02f,  0.02f, -0.02f,
		-0.02f,  0.02f, -0.02f,
		-0.02f, -0.02f,  0.02f,
		0.02f, -0.02f,  0.02f,
		0.02f,  0.02f,  0.02f,
		-0.02f,  0.02f,  0.02f
	};

	GLuint indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3,
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	mat4 model(1.0f);
    model = translate(model, vec3(trX, trY, trZ));
    // model = rotate(model, radians(90.0f), vec3(1.0, 0.0, 0.0));
    model = scale(model, vec3(scX, scY, scZ));
    shader.setMat4("model", model);

	shader.setVec3("color", vec3(0.5f,0.2f,0.2f));
    glBindVertexArray(VAO);
    GLuint isize = sizeof(indices)/sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	float cameraSpeed = 2.5 * deltaTime;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_W:
			if(cctvMode)
				break;
			cameraPos += cameraSpeed * cameraFront;
			break;
		case GLFW_KEY_S:
			if(cctvMode)
				break;
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case GLFW_KEY_A:
			if(cctvMode)
				break;
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case GLFW_KEY_D:
			if(cctvMode)
				break;
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case GLFW_KEY_R:
			cameraPos = cPosInit;
			cameraFront = cFrontInit;
			cameraUp = cUpInit;
			fovy = 45.0f;
			firstFrame = true;
			cctvMode=false;
			gscale = vec3(1.0f,1.0f,1.0f);
			break;
		case GLFW_KEY_Q:
			if(cctvMode){
				cctvMode=false;
				cameraPos = cPosInit;
				cameraFront = cFrontInit;
				cameraUp = cUpInit;
				fovy = 45.0f;
				firstFrame = true;
				gscale = vec3(1.0f,1.0f,1.0f);
			}
			else{
				cctvMode=true;
				cameraPos = vec3(1.74f, 0.85f, 0.0f);
				cameraFront = vec3(-1.0f, -0.5f, 0.0f);
				cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				fovy = 100.0f;
				gscale = vec3(0.299, 0.587, 0.114);
			}
			break;
		}
	}
}

void mouseCallBack(GLFWwindow* window, double xpos, double ypos) 
{
	if(cctvMode)
		return;

	if (firstFrame)
	{
		lastX = xpos;
		lastY = ypos;
		firstFrame = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	if(cctvMode)
		return;

	double  drpt = 2.0;
	double angle = drpt * yoffset;

	float cosA = glm::cos(glm::radians(angle));
	float sinA = -1*glm::sin(glm::radians(angle));

	glm::vec3 cameraDirection = cameraPos - glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));

	cameraRight = glm::normalize(cosA*cameraRight + sinA*cameraUp);
	cameraUp = glm::cross(cameraDirection,cameraRight);
}

void printHelp(){
	printf("----------Interactive User Guide-------------\n");
	printf("1.W to move forward/zoom in\n");
	printf("2.S to move backward/zoom out\n");
	printf("3.A to strafe left\n");
	printf("4.D to strafe right\n");
	printf("5.Move mouse to pitch and yaw\n");
	printf("6.Scroll mouse to roll\n");
	printf("7.Q to switch between CCTV and normal view\n");
	printf("8.R to reset camera position\n");
	printf("---------------------------------------------\n");
}