#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

float camera_rotation_angle = 90;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
//	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
//	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
//	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
  //  fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


float distance ( float x1, float y1, float x2, float y2 ) {
    //printf("%lf, %lf, %lf, %lf\n",x1, y2, x2, y2 );

  return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
}

float timeStart;

class Rectangle
{
public:
  VAO* rectangle; 
  float topLeftX, topLeftY, bottomRightX, bottomRightY, clR, clG, clB;
  Rectangle(float tlX, float tlY, float brX, float brY, float cR, float cG, float cB) {
    topLeftX = tlX;
    topLeftY = tlY;
    bottomRightX = brX;
    bottomRightY = brY;
    clR = cR;
    clG = cG;
    clB = cB;
  }

  void reset(float tlX, float tlY, float brX, float brY, float cR, float cG, float cB) {
   // cout << "reset called successfully" << endl;
    topLeftX = tlX;
    topLeftY = tlY;
    bottomRightX = brX;
    bottomRightY = brY;
    clR = cR;
    clG = cG;
    clB = cB;
  }

  void create () {
    GLfloat vertex_buffer_data [] = {
      topLeftX, topLeftY, 0,
      topLeftX, bottomRightY, 0,
      bottomRightX, bottomRightY, 0,

      bottomRightX, bottomRightY, 0,
      bottomRightX, topLeftY, 0,
      topLeftX, topLeftY, 0
    };

    GLfloat color_buffer_data [] = {
      clR, clG, clB,
      clR, clG, clB,
      clR, clG, clB,
      
      clR, clG, clB,
      clR, clG, clB,
      clR, clG, clB
    };

    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  }

  void collide();

  void draw (){
    glm::mat4 VP = Matrices.projection * Matrices.view;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 MVP;  // MVP = Projection * View * Model
    
    //glm::mat4 translate;
    //translate = glm::translate(glm::vec3(posX, posY, 0.0f));
    
    //Matrices.model *= translate;
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(rectangle);
  }

};

class canon
{
public:
  VAO *canonCircle, *canonRectangle;
  float canonCircleRadius, canonCircleCentreX, canonCircleCentreY;
  float canonRectangleX1, canonRectangleY1, canonRectangleX2, canonRectangleY2;
  float angle;

  void reset() {
    canonCircleRadius = 50.0f;
    canonCircleCentreX = -850;
    canonCircleCentreY = -350;
    canonRectangleX1 = canonCircleCentreX + canonCircleRadius + 40.0f;
    canonRectangleY1 = canonCircleCentreY;
    canonRectangleX2 = canonCircleCentreX+canonCircleRadius+40.0f;
    canonRectangleY2=canonCircleCentreY+30.0f;
    angle = 30;

  }
  void create () {
    int numTriangle = 150;
    GLfloat vertex_buffer_data [9*numTriangle];
    GLfloat color_buffer_data [9*numTriangle];
    int i, j=0;
    for(i=0; i<numTriangle; i++) {
      float theta = 2.0f * 3.1415926f * float(i)/float(numTriangle);
      float x = canonCircleRadius * cosf(theta);
      float y = canonCircleRadius * sinf(theta);
      vertex_buffer_data[j] = x+canonCircleCentreX;
      vertex_buffer_data[j+1] = y+canonCircleCentreY;
      vertex_buffer_data[j+2] = 0;
      vertex_buffer_data[j+3] = canonCircleCentreX;
      vertex_buffer_data[j+4] = canonCircleCentreY;
      vertex_buffer_data[j+5] = 0;
      theta = 2.0f * 3.1415926f * float(i+1)/float(numTriangle);
      x = canonCircleRadius * cosf(theta);
      y = canonCircleRadius * sinf(theta);
      vertex_buffer_data[j+6] = x+canonCircleCentreX;
      vertex_buffer_data[j+7] = y+canonCircleCentreY;
      vertex_buffer_data[j+8] = 0;
      color_buffer_data[j]= 0.5f;
      color_buffer_data[j+1] = 0;
      color_buffer_data[j+2] = 0.5f;
      color_buffer_data[j+3] = 0.5f;
      color_buffer_data[j+4] = 0;
      color_buffer_data[j+5] = 0.5f;
      color_buffer_data[j+6] = 0.5f;
      color_buffer_data[j+7] = 0;
      color_buffer_data[j+8] = 0.5f;
      j=j+9;
    }
    canonCircle = create3DObject(GL_TRIANGLES, 3*numTriangle, vertex_buffer_data, color_buffer_data, GL_FILL);
    
    GLfloat vertex1_buffer_data [] = {
      canonCircleCentreX, canonCircleCentreY, 0,
      canonCircleCentreX + canonCircleRadius + 40.0f, canonCircleCentreY, 0,
      canonCircleCentreX, canonCircleCentreY+30.0f, 0,

      canonCircleCentreX+40.0f+ canonCircleRadius, canonCircleCentreY, 0,
      canonCircleCentreX+40.0f+ canonCircleRadius, canonCircleCentreY+30.0f, 0,
      canonCircleCentreX, canonCircleCentreY+30.0f, 0
    };

    GLfloat color1_buffer_data [] ={
      0.5f, 0, 0.5f,
      0.5f, 0, 0.5f,
      0.5f, 0, 0.5f,

      0.5f, 0, 0.5f,
      0.5f, 0, 0.5f,
      0.5f, 0, 0.5f
    };

    canonRectangle = create3DObject(GL_TRIANGLES, 6, vertex1_buffer_data, color1_buffer_data, GL_FILL);
  }

  void draw() {

    //glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    //glm::vec3 target (0, 0, 0);
    //glm::vec3 up (0, 1, 0);

    //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); 
    glm::mat4 VP = Matrices.projection * Matrices.view;
    //Matrices.model = glm::mat4(1.0f);
    glm::mat4 MVP;  // MVP = Projection * View * Model
    MVP = VP*Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(canonCircle);


    glm::mat4 translate = glm::translate(glm::vec3(-canonCircleCentreX, -canonCircleCentreY +15.0f, 0.0f));
    glm::mat4 rotate = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
    glm::mat4 translate_back = glm::translate(glm::vec3(canonCircleCentreX, canonCircleCentreY -15.0f, 0.0f));
    //Matrices.model *= translate*rotate*translate_back;
    Matrices.model *= translate_back*rotate*translate;
    MVP = VP*Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(canonRectangle);
    float *ptr;
    ptr= &(Matrices.model[0][0]);
    canonRectangleX1 = ptr[0]*canonRectangleX1 + ptr[4]*canonRectangleY1 + ptr[12];
    canonRectangleY1 = ptr[1]*canonRectangleX1 + ptr[5]*canonRectangleY1 + ptr[13];
    canonRectangleX2 = ptr[0]*canonRectangleX2 + ptr[4]*canonRectangleY2 + ptr[12];
    canonRectangleY2 = ptr[1]*canonRectangleX2 + ptr[5]*canonRectangleY2 + ptr[13];   
  } 
  /* data */
} Canon;

int totalScore = 0;

class target
{
public:

  VAO* circle;
  float radius, posX, posY, score, colorR, colorG, colorB, velX, velY;

  target(float r, float pX, float pY, float s, float cR, float cG, float cB) {
    radius = r;
    posX = pX;
    posY = pY;
    score = s;
    colorR = cR;
    colorG = cG;
    colorB = cB;
  }

  void collide( float rad, float vX, float vY, float pX, float pY ) {
    if( sqrt(distance(pX, pY, posX, posY)) < (rad + radius) ) {
      //cout << "hello" << endl;
  //    printf("%lf, %lf", sqrt(distance(pX,pY, posX, posY)), rad+radius);
      totalScore += score;
    }
  }

  void create () {
    int numTriangle = 150;
    GLfloat vertex_buffer_data [9*numTriangle];
    GLfloat color_buffer_data [9*numTriangle];
    int i, j=0;
    for(i=0; i<numTriangle; i++) {
      float theta = 2.0f * 3.1415926f * float(i)/float(numTriangle);
      float x = radius * cosf(theta);
      float y = radius * sinf(theta);
      vertex_buffer_data[j] = x;
      vertex_buffer_data[j+1] = y;
      vertex_buffer_data[j+2] = 0;
      vertex_buffer_data[j+3] = 0;
      vertex_buffer_data[j+4] = 0;
      vertex_buffer_data[j+5] = 0;
      theta = 2.0f * 3.1415926f * float(i+1)/float(numTriangle);
      x = radius * cosf(theta);
      y = radius * sinf(theta);
      vertex_buffer_data[j+6] = x;
      vertex_buffer_data[j+7] = y;
      vertex_buffer_data[j+8] = 0;
      color_buffer_data[j]   = colorR;
      color_buffer_data[j+1] = colorG;
      color_buffer_data[j+2] = colorB;
      color_buffer_data[j+3] = colorR;
      color_buffer_data[j+4] = colorG;
      color_buffer_data[j+5] = colorB;
      color_buffer_data[j+6] = colorR;
      color_buffer_data[j+7] = colorG;
      color_buffer_data[j+8] = colorB;
      j=j+9;
    }
    circle = create3DObject(GL_TRIANGLES, 3*numTriangle, vertex_buffer_data, color_buffer_data, GL_FILL);
  } 

  void draw() { 
    glm::mat4 VP = Matrices.projection * Matrices.view;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 MVP;  // MVP = Projection * View * Model
    
    glm::mat4 translate;
    translate = glm::translate(glm::vec3(posX, posY, 0.0f));
    
    Matrices.model *= translate;
    MVP = VP* Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);
  }
};

class ball
{
public:
  VAO *circle;
  float centreX, centreY, radius;
  float posX, posY;
  float initVel;
  float tempVel;
  float velX , velY, ballDirX;
  bool thrown;
  void reset() {
   // printf("ball reset function called properly \n");
    centreX = (Canon.canonRectangleX2/2 + Canon.canonRectangleX1/2);
    centreY = (Canon.canonRectangleY1/2 + Canon.canonRectangleY2/2);
    radius = 20.0f;
    initVel = 1000.0f;
    tempVel = 1000.0f;
    velX = 0;
    velY = 0;
    posX = centreX;
    posY = centreY;
    thrown = 0;
    ballDirX = 1;
  }
  void create () {
    int numTriangle = 150;
    GLfloat vertex_buffer_data [9*numTriangle];
    GLfloat color_buffer_data [9*numTriangle];
    int i, j=0;
    for(i=0; i<numTriangle; i++) {
      float theta = 2.0f * 3.1415926f * float(i)/float(numTriangle);
      float x = radius * cosf(theta);
      float y = radius * sinf(theta);
      vertex_buffer_data[j] = x;
      vertex_buffer_data[j+1] = y;
      vertex_buffer_data[j+2] = 0;
      vertex_buffer_data[j+3] = 0;
      vertex_buffer_data[j+4] = 0;
      vertex_buffer_data[j+5] = 0;
      theta = 2.0f * 3.1415926f * float(i+1)/float(numTriangle);
      x = radius * cosf(theta);
      y = radius * sinf(theta);
      vertex_buffer_data[j+6] = x;
      vertex_buffer_data[j+7] = y;
      vertex_buffer_data[j+8] = 0;
      color_buffer_data[j]= 1.0f;
      color_buffer_data[j+1] = 0;
      color_buffer_data[j+2] = 0;
      color_buffer_data[j+3] = 1.0f;
      color_buffer_data[j+4] = 0;
      color_buffer_data[j+5] = 0;
      color_buffer_data[j+6] = 1.0f;
      color_buffer_data[j+7] = 0;
      color_buffer_data[j+8] = 0;
      j=j+9;
    } 
    circle = create3DObject(GL_TRIANGLES, 3*numTriangle, vertex_buffer_data, color_buffer_data, GL_FILL);
  }

  void draw() {
    //glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    //glm::vec3 target (0, 0, 0);
    //glm::vec3 up (0, 1, 0);
    //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
   // if(thrown == 0) {
    //     centreX = (Canon.canonRectangleX2/2 + Canon.canonRectangleX1/2);
     //    centreY= (Canon.canonRectangleY1/2 + Canon.canonRectangleY2/2);

    //} 
    glm::mat4 VP = Matrices.projection * Matrices.view;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 MVP;  // MVP = Projection * View * Model
    //MVP = VP*Matrices.model;
    //glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    //draw3DObject(canonCircle);
    //cout << centreX << " " << centreY << endl;
  //  glm::mat4 translate = glm::translate(glm::vec3(centreX, centreY, 0.0f));
    glm::mat4 translate;
   // if(thrown ==0)
    //  translate = glm::translate(glm::vec3(centreX, -300, 0.0f));

    //else {
      translate = glm::translate(glm::vec3(posX, posY, 0.0f));
      //cout << posX << " " << posY << endl;
    //}
    //glm::mat4 rotate = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
    //glm::mat4 translate_back = glm::translate(glm::vec3(canonCircleCentreX, canonCircleCentreY -0.15f, 0.0f));
    //Matrices.model *= translate*rotate*translate_back;
    Matrices.model *= translate;
    MVP = VP* Matrices.model;
//0,4,12   1,5,13
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(thrown == 1) {
      draw3DObject(circle);
    }
   // float *ptr = &(Matrices.model[0][0]);
   // centreX = ptr[0]*centreX + ptr[4]*centreY + ptr[12];
   // centreY = ptr[1]*centreX + ptr[5]*centreY + ptr[13];
  //  cout << "$"<<centreX << " $" << centreY << endl;
  }

} Ball;

void Rectangle::collide() {
  //cout << "HELLO\n";
    if( topLeftX - Ball.posX <= Ball.radius and Ball.posY >= bottomRightY  and Ball.posY <= topLeftY and bottomRightX - Ball.posX >= Ball.radius) {
    //  cout << "hello\n";
      timeStart = glfwGetTime();
      Ball.ballDirX = -Ball.ballDirX;
      Ball.tempVel = 0.7*Ball.tempVel;
      Ball.velX = Ball.tempVel*cosf(Canon.angle*(M_PI/180));
      Ball.velY = Ball.tempVel*sinf(Canon.angle*(M_PI/180));
      Ball.posX = topLeftX - Ball.radius -10;
      Ball.centreX = Ball.posX;
      Ball.centreY = Ball.posY;
    //  cout << topLeftX << "$" << endl;
    //  cout << Ball.posX << "^" << endl;
     // cout << Ball.velX << " " << Ball.velY << endl;
    }
}
//global variables
float g = 10, numOfBalls = 5;
target  mainTarget(100, 800, 300, 500, 0.5f, 0.5f, 0.5f);
Rectangle obstacle1(250, 500, 350, 200, 1.0f, 1.0f, 0.0f);
Rectangle obstacle2(250, 0, 350, -400, 1.0f, 1.0f, 0.0f);
Rectangle powerIndicator(-980, 0, -920, -400, 0.0f, 1.0f, 0.0f);
/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

void myCursorFunc(GLFWwindow* window, double xpos, double ypos) {
  float x = -(500 - xpos);
  float y = (500 - ypos);
  float temp = atan((y - Canon.canonCircleCentreY)/(x-Canon.canonCircleCentreX));
  temp = (180/M_PI)*temp;
  Canon.angle = temp;
  cout << temp<< endl;
}
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_A:
                Canon.angle += 2;
                break;
            case GLFW_KEY_B:
                Canon.angle -= 2;
                  break;
            case GLFW_KEY_SPACE:
              if(Ball.thrown == 0) {
               // cout << Ball.initVel;
                timeStart =glfwGetTime();
                Ball.velX = Ball.initVel*cosf(Canon.angle*(M_PI/180));
                Ball.velY = Ball.initVel*sinf(Canon.angle*(M_PI/180)) ;
              //  Ball.startX = Ball.centreX;
              //  Ball.startY = Ball.centreY;
                Ball.thrown = 1;
              }
              break;
            case GLFW_KEY_F:
                Ball.initVel += 5;
                powerIndicator.topLeftY += 3;
                if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
                  powerIndicator.clG = 0.5f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB = 0.0f;
                }
                else if(Ball.initVel >1500) {
                  powerIndicator.clG = 0.0f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB =0.0f;
                }
                else {
                  powerIndicator.clG = 1.0f;
                  powerIndicator.clR = 0.0f;
                  powerIndicator.clB = 0.0f;
                }
                powerIndicator.create();

                break;
            case GLFW_KEY_S:
                Ball.initVel -= 5;
                powerIndicator.topLeftY -= 3;
                 if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
                  powerIndicator.clG = 0.5f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB = 0.0f;
                }
                else if(Ball.initVel >1500) {
                  powerIndicator.clG = 0.0f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB =0.0f;
                }
                else {
                  powerIndicator.clG = 1.0f;
                  powerIndicator.clR = 0.0f;
                  powerIndicator.clB = 0.0f;
                }
                powerIndicator.create();

                break;
            default:
                break;
        }
    }
    else if (action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_A:
                Canon.angle += 2;
                break;
            case GLFW_KEY_B:
                Canon.angle -= 2;
                break;
            case GLFW_KEY_F:
                Ball.initVel += 4;
                powerIndicator.topLeftY += 2;
                if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
                  powerIndicator.clG = 0.5f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB = 0.0f;
                }
                else if(Ball.initVel >1500) {
                  powerIndicator.clG = 0.0f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB =0.0f;
                }
                else {
                  powerIndicator.clG = 1.0f;
                  powerIndicator.clR = 0.0f;
                  powerIndicator.clB = 0.0f;
                }
                powerIndicator.create();

                break;
            case GLFW_KEY_S:
                Ball.initVel -= 4;
                powerIndicator.topLeftY -= 2;
                if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
                  powerIndicator.clG = 0.5f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB = 0.0f;
                }
                else if(Ball.initVel >1500) {
                  powerIndicator.clG = 0.0f;
                  powerIndicator.clR = 1.0f;
                  powerIndicator.clB =0.0f;
                }
                else {
                  powerIndicator.clG = 1.0f;
                  powerIndicator.clR = 0.0f;
                  powerIndicator.clB = 0.0f;
                }
                powerIndicator.create();
                break;
            default:
                break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  cout << yoffset << endl;
  if( yoffset == 1) {
    Ball.initVel += 4;
    powerIndicator.topLeftY += 2;
    if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
      powerIndicator.clG = 0.5f;
      powerIndicator.clR = 1.0f;
      powerIndicator.clB = 0.0f;
    }
    else if(Ball.initVel >1500) {
      powerIndicator.clG = 0.0f;
      powerIndicator.clR = 1.0f;
      powerIndicator.clB =0.0f;
    }
    else {
      powerIndicator.clG = 1.0f;
      powerIndicator.clR = 0.0f;
      powerIndicator.clB = 0.0f;
    }
    powerIndicator.create();
  }

  else if(yoffset == -1) {
    Ball.initVel -= 4;
    powerIndicator.topLeftY -= 2;
    if(Ball.initVel >= 1200 and Ball.initVel <= 1500 ) {
      powerIndicator.clG = 0.5f;
      powerIndicator.clR = 1.0f;
      powerIndicator.clB = 0.0f;
    }
    else if(Ball.initVel >1500) {
      powerIndicator.clG = 0.0f;
      powerIndicator.clR = 1.0f;
      powerIndicator.clB =0.0f;
    }
    else {
      powerIndicator.clG = 1.0f;
      powerIndicator.clR = 0.0f;
      powerIndicator.clB = 0.0f;
    }
    powerIndicator.create();

  }
}
/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
          if (action == GLFW_RELEASE) {
              if(Ball.thrown == 0) {
              // cout << Ball.initVel;
              timeStart =glfwGetTime();
              Ball.velX = Ball.initVel*cosf(Canon.angle*(M_PI/180));
              Ball.velY = Ball.initVel*sinf(Canon.angle*(M_PI/180)) ;
              //  Ball.startX = Ball.centreX;
              //  Ball.startY = Ball.centreY;
              Ball.thrown = 1;
            }
          }
          break;
      case GLFW_MOUSE_BUTTON_RIGHT:
          if (action == GLFW_RELEASE) {
              rectangle_rot_dir *= -1;
          }
          break;
      default:
          break;
    }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-1000.0f, 1000.0f, -600.0f, 600.0f, 0.1f, 500.0f);
}

VAO *rectangle;
//Customized create functions
VAO *ground, *background;
void createBackground() {
  static const GLfloat vertex_buffer_data [] ={
    1000,600,0,
    1000,-600,0,
    -1000,600,0,

    -1000,600,0,
    -1000,-600,0,
    1000,-600,0
  };

  static const GLfloat color_buffer_data [] ={
    1,1,1,
    1,1,1,
    1,1,1,

    1,1,1,
    1,1,1,
    1,1,1
  };
  background = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createGround() {
  static const GLfloat vertex_buffer_data [] = {
    -1000,-400,0,
    -1000,-600,0,
    1000,-600,0,

    -1000,-400,0,
    1000,-400,0,
    1000,-600,0
  };
  static const GLfloat color_buffer_data [] = {
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0
  };
  ground = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -50,-50,0, // vertex 1
    50,-50,0, // vertex 2
    50,50,0, // vertex 3

    -50,-50,0, // vertex 3
    -50,50,0, // vertex 4
    50,50,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void checkCollisionWithGround() {
 // cout << Ball.centreY <<endl;
    if(Ball.posY - Ball.radius <= -400) {
  //    cout << Ball.velY << " balle" <<endl;
      //Ball.centreY = -380.0f;
      if(Ball.posX - Ball.centreX >= 20) {
        timeStart = glfwGetTime();
        Ball.velY = 0.7*Ball.tempVel*sinf(Canon.angle*(M_PI/180 ));
        Ball.tempVel = 0.7*Ball.tempVel;
        Ball.velX = 0.7*Ball.tempVel*cosf(Canon.angle*(M_PI/180 ));
       // cout << Ball.velY;
        Ball.centreX = Ball.posX;
        Ball.centreY = Ball.posY;
       // if(Ball.thrown == 1)
        //  if(Ball.centreY <= -400 or Ball.centreX <= -980 or Ball.centreY >= 980) {
         //   cout << "hellelujah" << endl;
          // Ball.reset();
          //}
      }
    }

}

float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);
  MVP = VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(background);
  draw3DObject(ground);
  checkCollisionWithGround();
  if(Ball.posX > 150 and Ball.posX < 600) {
    obstacle2.collide();
    obstacle1.collide();
  }
  if(Ball.thrown == 1)
    mainTarget.collide(Ball.radius, Ball.velX, Ball.velY, Ball.posX, Ball.posY);
  Canon.draw();
  Ball.draw();
  mainTarget.draw();
  obstacle1.draw();
  obstacle2.draw();
  powerIndicator.draw();
  //Matrices.model = glm::mat4(1.0f);
  //MVP = VP*Matrices.model;
  //glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  //draw3DObject(Canon.canonCircle);

  /* Render your scene */

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
 // draw3DObject(triangle);F

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  //Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
 // draw3DObject(rectangle);

  // Increment angles
  float increments = 1;

  //resetting conditions for the ball
  if( Ball.posX >= 980 or Ball.posX <= -980 or Ball.tempVel <= 20 or Ball.posY <= -400) {
    if(Ball.thrown == 1) {
  //    cout << Ball.velX<< endl;
      //cout <<"balle \n";
      powerIndicator.reset(-980, 0, -920, -400, 0.0f, 1.0f, 0.0f);
      powerIndicator.create();
      if(numOfBalls > 0) {
        numOfBalls--;
        cout << "NO. of chances left = " << numOfBalls << endl;
        cout << "Score is = " << totalScore << endl<<endl;
        Ball.reset();
        Ball.create();
      }
    }
  }
  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
  GLFWwindow* window; // window desciptor/handle

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
      exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

  if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
  glfwSetFramebufferSizeCallback(window, reshapeWindow);
  glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
  glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
  glfwSetKeyCallback(window, keyboard);      // general keyboard input
  glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
  glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

  return window;
}



/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
   //obstacle(float r, float pX, float pY, float s, float cR, float cG, float cB) {
  Canon.reset();
  Ball.reset();
  powerIndicator.reset(-980, 0, -920, -400, 0.0f, 1.0f, 0.0f);

  
	createRectangle ();
  createBackground();
	createGround();
  Canon.create();
  Ball.create();
  mainTarget.create();
  obstacle1.create();
  obstacle2.create();
  powerIndicator.create();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 600;

  GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
      draw();

        // Swap Frame Buffer in double buffering
      glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
      glfwPollEvents();

      glfwSetCursorPosCallback(window, myCursorFunc);

      glfwSetScrollCallback(window, scroll_callback);

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
      current_time = glfwGetTime(); // Time in seconds
      if ((current_time - last_update_time) >= 0.000000001  ) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
          float t = current_time - timeStart;
          if(Ball.thrown == 1) {
              //cout << "$" << Ball.velY;
            Ball.posX = Ball.centreX + Ball.ballDirX*Ball.velX*t  ;
            Ball.posY = Ball.centreY + Ball.velY*t  ;
     //         cout << "$" << Ball.velY << endl;
            Ball.velY -= g*t;
           //   cout << Ball.velX << " " << Ball.velY << endl;
          }
          
        last_update_time = current_time;
      }
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
