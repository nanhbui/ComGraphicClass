#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"

using namespace std;

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;
Camera camera;

DirectionalLight dLight(1.0f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f);

GLfloat off = 1, on=0;
GLuint textureID;
GLuint textureID1;
GLuint textureID2;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat alpha = 0.0, theta = 0.0, axis_x = 0.0, axis_y = 0.0, Calpha = 360.0, C_hr_alpha = 360.0;
GLboolean l_on = true;
const int width = 800;
const int height = 600;
GLboolean amb = true, spec = true, dif = true;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";
// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

bool l_on1 = false;
bool l_on2 = false;
bool l_on3 = false;
bool fanRotate = false;
bool isMovingUp = false;
bool isFloat = true;
bool cameraFollowGhost = false; // Camera theo dõi ghost
bool showSquare = false;

double spt_cutoff = 40;

float rot = 0;

GLfloat eyeX = 0;
GLfloat eyeY = 10;
GLfloat eyeZ = 10;
GLfloat lookX = 0;
GLfloat lookY = 10;
GLfloat lookZ = 0;
GLfloat savedEyeX, savedEyeY, savedEyeZ;
GLfloat savedLookX, savedLookY, savedLookZ;
GLfloat savedEyeGX, savedEyeGY, savedEyeGZ;
GLfloat savedLookGX, savedLookGY, savedLookGZ;
GLfloat ghostX = 0, ghostZ = -10; // Vị trí X và Z ban đầu của ghost
GLfloat ghostAngle = 0.0f; // Hướng của ghost
GLfloat boardPositionY = 1.0f; // Vị trí ban đầu của bảng
GLfloat movementSpeed = 0.0006f; // Tốc độ di chuyển của bảng
GLfloat ghostPositionY = 1.0f; // Vị trí ban đầu của ma
GLfloat movementSpeed1 = 0.0006f; // Tốc độ di chuyển của ma
GLfloat squareSize = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static GLfloat v_Cube[8][3] =
{
    {0,0,0},
    {0,0,1},
    {0,1,0},
    {0,1,1},

    {1,0,0},
    {1,0,1},
    {1,1,0},
    {1,1,1}
};
static GLubyte quadIndices[6][4] =
{
   {0,2,6,4},
    {1,5,7,3},
    {0,4,5,1},
    {2,3,7,6},
    {0,1,3,2},
    {4,6,7,5}
};
static void getNormal3p(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2, GLfloat x3, GLfloat y3, GLfloat z3)
{
    GLfloat Ux, Uy, Uz, Vx, Vy, Vz, Nx, Ny, Nz;

    Ux = x2 - x1;
    Uy = y2 - y1;
    Uz = z2 - z1;

    Vx = x3 - x1;
    Vy = y3 - y1;
    Vz = z3 - z1;

    Nx = Uy * Vz - Uz * Vy;
    Ny = Uz * Vx - Ux * Vz;
    Nz = Ux * Vy - Uy * Vx;

    glNormal3f(Nx, Ny, Nz);
}

void cube(GLfloat colr1, GLfloat colr2, GLfloat colr3)
{
    GLfloat cube_no[] = { 0, 0, 0, 1.0 };
    GLfloat cube_amb[] = { colr1 * 0.3, colr2 * 0.3, colr3 * 0.3, 1 };
    GLfloat cube_dif[] = { colr1, colr2, colr3, 1 };
    GLfloat cube_spec[] = { 1, 1, 1, 1 };
    GLfloat cube_sh[] = { 100 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, cube_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, cube_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, cube_sh);

    glBegin(GL_QUADS);
    for (GLint i = 0; i < 6; i++)
    {
        getNormal3p(v_Cube[quadIndices[i][0]][0], v_Cube[quadIndices[i][0]][1], v_Cube[quadIndices[i][0]][2],
            v_Cube[quadIndices[i][1]][0], v_Cube[quadIndices[i][1]][1], v_Cube[quadIndices[i][1]][2],
            v_Cube[quadIndices[i][2]][0], v_Cube[quadIndices[i][2]][1], v_Cube[quadIndices[i][2]][2]);
        for (GLint j = 0; j < 4; j++)
        {
            glVertex3fv(&v_Cube[quadIndices[i][j]][0]);
        }
    }
    glEnd();
}
void TexturedSquare(GLfloat colr1, GLfloat colr2, GLfloat colr3,GLuint textureID1) {
    GLfloat square_amb[] = { colr1 * 0.3, colr2 * 0.3, colr3 * 0.3, 1 };
    GLfloat square_dif[] = { colr1, colr2, colr3, 1 };
    GLfloat square_spec[] = { 1, 1, 1, 1 };
    GLfloat square_sh[] = { 10 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, square_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, square_dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, square_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, square_sh);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID1);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.5f, 0.5f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}
void sphere(GLfloat colr1, GLfloat colr2, GLfloat colr3, GLfloat radius, GLint slices, GLint stacks)
{
    GLfloat sphere_no[] = { 0, 0, 0, 1.0 };
    GLfloat sphere_amb[] = { colr1 * 0.3, colr2 * 0.3, colr3 * 0.3, 1 };
    GLfloat sphere_dif[] = { colr1, colr2, colr3, 1 };
    GLfloat sphere_spec[] = { 1, 1, 1, 1 };
    GLfloat sphere_sh[] = { 10 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, sphere_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sphere_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, sphere_sh);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}
GLuint loadTexture(const char* filename)
{
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), width, height, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    return textureID;
}
void sphereWithTexture(GLfloat colr1, GLfloat colr2, GLfloat colr3, GLfloat radius, GLint slices, GLint stacks, GLuint textureID)
{
    GLfloat sphere_no[] = { 0, 0, 0, 1.0 };
    GLfloat sphere_amb[] = { colr1 * 0.3, colr2 * 0.3, colr3 * 0.3, 1 };
    GLfloat sphere_dif[] = { colr1, colr2, colr3, 1 };
    GLfloat sphere_spec[] = { 1, 1, 1, 1 };
    GLfloat sphere_sh[] = { 10 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, sphere_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sphere_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, sphere_sh);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);

    glDisable(GL_TEXTURE_2D);
}

void table()
{
    float length = 1;
    float height = 3;
    float width = 0.5;

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(0, 0, 0);
    cube(0.0, 0.0, 0.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(4, 0, 0);
    cube(0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(0, 0, 3);
    cube(0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(4, 0, 3);
    cube(0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glScalef(length * 6, height / 6, width * 5);
    glTranslatef(0, 6, 0);
    cube(1, .8, .4);
    glPopMatrix();
}
void chair()
{
    float length = .5;
    float height = 2;
    float width = .5;

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(.5, 0, -2);
    cube(1, 0.8, 0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(3.5, 0, -2);
    cube(1, 0.8, 0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(.5, 0, -4);
    cube(1, 0.8, 0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length, height, width);
    glTranslatef(3.5, 0, -4);
    cube(1, 0.8, 0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length * 4, height / 6, width * 4);
    glTranslatef(0.15, 6, -1.2);
    cube(1, 0.7, 0.4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(length / 2, 0, 0);
    glScalef(length * 4, height * 1.2, width);
    glTranslatef(.15, 1, -5);
    cube(1, 0.7, 0.4);
    glPopMatrix();
}
void full_set()
{
    glPushMatrix();
    table();
    glPopMatrix();

    glPushMatrix();
    chair();
    glPopMatrix();
}
void full_set_chair_table()
{
    glPushMatrix();
    glTranslatef(3, 0, 0);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(9, 0, 0);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-8, 0, 0);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14, 0, 0);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 0, -6);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(9, 0, -6);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-8, 0, -6);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14, 0, -6);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 0, -12);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(9, 0, -12);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-8, 0, -12);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14, 0, -12);
    full_set();
    glPopMatrix();

    glPushMatrix();
    glScalef(2, 2, 1);
    glTranslatef(2, 0, 10.5);
    table();
    glPopMatrix();
}
void fan_face()
{
    glPushMatrix();
    glScalef(2, .5, 1.8);
    glTranslatef(-0.4, 30, -.4);
    cube(0.25, 0.875, 0.815);
    glPopMatrix();
}
void stand()
{
    glPushMatrix();
    glScalef(.5, 5, .5);
    glTranslatef(0, 3, 0);
    cube(0.25, 0.875, 0.815);
    glPopMatrix();
}
void leg()
{
    glPushMatrix();
    glScalef(5, 0.07, 1.8);
    glTranslatef(0, 217, -0.3);
    cube(0.25, 0.875, 0.815);
    glPopMatrix();
}
void fan_set()
{
    glPushMatrix();
    glPushMatrix();
    fan_face();
    glPopMatrix();

    glPushMatrix();
    leg();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-5, 0.0, 0);
    leg();
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(-6, 0, 0);
    leg();
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(0, 0, 0);
    leg();
    glPopMatrix();
}
void fan()
{
    glPushMatrix();
    glRotatef(alpha, 0, .1, 0);
    fan_set();
    glPopMatrix();

    glPushMatrix();
    stand();
    glPopMatrix();
}
void fan_full_set()
{
    glPushMatrix();
    glTranslatef(7, 0, 3);
    fan();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-7, 0, 3);
    fan();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(7, 0, -9);
    fan();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-7, 0, -9);
    fan();
    glPopMatrix();
}
void caugiao() {
    glPushMatrix();
    glScalef(4, 4, 0.2);
    glTranslatef(0, 0, 0);
    cube(0.5, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.3, 1.3, 0.5);
    glTranslatef(0.3, 0.3, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.3, 1.3, 0.5);
    glTranslatef(1.8, 0.3, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.3, 1.3, 0.5);
    glTranslatef(0.3, 1.6, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.3, 1.3, 0.5);
    glTranslatef(1.8, 1.6, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1, 1, 0.4);
    sphere(off, on, 0, 0.4, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 1, 0.4);
    sphere(off, on, 0, 0.4, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1, 2.7, 0.4);
    sphere(off, on, 0, 0.4, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 2.7, 0.4);
    sphere(off, on, 0, 0.4, 32, 32);
    glPopMatrix();
}
void caugiaoFull()
{
    glPushMatrix();
    glTranslatef(-15,5,-19);
    caugiao();
    glPopMatrix();
}
void ball() {
    glPushMatrix();
    glTranslatef(-4, 16.5, -17);
    sphere(0,1,0,0.2,32,32);
    glPopMatrix();
}
void ball2() {
    glPushMatrix();
    glTranslatef(8, 8, 12);
    sphereWithTexture(1, 1, 1, 1, 32, 32,textureID);
    glPopMatrix();
}
void bucgiang()
{
    glPushMatrix();

    GLfloat bucgiang_height = 2, bucgiang_width = 35, bucgiang_length = 10;

    glPushMatrix();
    glScalef(bucgiang_width, bucgiang_height, bucgiang_length);
    glTranslatef(-0.4555, 0, 1);
    cube(0.8, 0.8, 0.8);
    glPopMatrix();



    glPopMatrix();
}
void board()
{
    glPushMatrix();
    glScalef(20, 8, 0.1);
    glTranslatef(-0.6, .6, 188);
    cube(0, 0.3, 0);
    glPopMatrix();

    glPushMatrix();
    glScalef(20.5, 8.5, 0.2);
    glTranslatef(-0.6, 0.54, 94.8);
    cube(0.6, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glScalef(3, 1, 2);
    glTranslatef(3, 15, 8.5);
    cube(0, 0, 0);
    glPopMatrix();
}
void window()
{
    // mặt kính
    glPushMatrix();
    glScalef(0.1, 10, 10);
    glTranslatef(-140, .3, -.5);
    cube(.5, 1, 1);
    glPopMatrix();

    // cột giữa
    glPushMatrix();
    glScalef(0.1, 10, .1);
    glTranslatef(-139, .3, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1, 0.1, 10);
    glTranslatef(-139, 100, -.5);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1, 0.1, 10);
    glTranslatef(-139, 130, -.5);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1, 0.1, 10);
    glTranslatef(-139, 30, -.5);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1, 10, .1);
    glTranslatef(-139, .3, -50);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.1, 10, .1);
    glTranslatef(-139, .3, 50);
    cube(1, 1, 1);
    glPopMatrix();
}
void window1()
{
    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(-4, 3, 8);
    window();
    glPopMatrix();
}
void window2()
{
    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(-4, 3, -8);
    window();
    glPopMatrix();
}
void projector()
{
    // than
    glPushMatrix();
    glScalef(2, .7, 1.8);
    glTranslatef(-0.43, 18, -.4);
    cube(1, 1, 1);
    glPopMatrix();
    //chan
    glPushMatrix();
    glScalef(.3, 6, .3);
    glTranslatef(0, 2.2, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(.68, .68, .68);
    glTranslatef(0, 18.45, 1);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(.60, .60, .60);
    glTranslatef(0.05, 20.9, 1.4);
    cube(0, 0, 0);
    glPopMatrix();
}
void image() {
    glPushMatrix();
    glScalef(squareSize, squareSize, 5);
    glTranslatef(0, 1.5, 3.5888);
    TexturedSquare(1, 1, 1,textureID1);
    glPopMatrix();
}
void image2() {
    glPushMatrix();
    glScalef(40, 40, 40);
    glTranslatef(0, 0.4739, 0);
    glRotatef(90, .1, 0, 0);
    TexturedSquare(1, 1, 1, textureID2);
    glPopMatrix();
}
void projector_board()
{
    glPushMatrix();
    glScalef(9, 40, 0.1);
    glTranslatef(-0.5, boardPositionY, 180);
    cube(0.5, 0.5, 0.5);
    glPopMatrix();
}
void ghost()
{

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(0, 7, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.3, 6.9, 1.3);
    glTranslatef(-0.1, 0, -0.1);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.56, 6.5, 1.3);
    glTranslatef(-0.16, 0.3, -0.26);
    cube(0, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.3, 0.3, 0.3);
    glTranslatef(0.2, 25, 2.6);
    cube(1, 0, 0);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.3, 0.3, 0.3);
    glTranslatef(2.1, 25, 2.6);
    cube(1, 0, 0);
    glPopMatrix();
}
void ghostFull() {
    glPushMatrix();
    glTranslatef(ghostX, ghostPositionY, ghostZ);
    glRotatef(-ghostAngle,0,.1,0);
    ghost();
    glPopMatrix();
}
void AC()
{
    glPushMatrix();
    glScalef(6, 2, 2);
    glTranslatef(-1.5, 8, -9.5);
    cube(.8, .8, .8);
    glPopMatrix();
}
void bulb()
{
    glPushMatrix();
    glScalef(2, .5, 2);
    glTranslatef(0, 10.5, 0);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(.2, .3, .2);
    glTranslatef(0.7, 18, 0.7);
    cube(1, 0, 0);
    glPopMatrix();
}
void bulb_set()
{
    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(-12, 13.5, -12);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(12, 13.5, -12);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(-12, 13.5, 12);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(12, 13.5, 12);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(-5, 13.5, -5);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(5, 13.5, -5);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(-5, 13.5, 5);
    bulb();
    glPopMatrix();

    glPushMatrix();
    glScalef(1, 1, 1);
    glTranslatef(5, 13.5, 5);
    bulb();
    glPopMatrix();
}
void door()
{
    glPushMatrix();
    glScalef(.2, 15, 3.5);
    glTranslatef(-95, 0, 3);
    cube(0.5, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glScalef(.2, 15, 3.5);
    glTranslatef(-95, 0, 1.99);
    cube(0.5, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glScalef(.2, 14.6, 3.1);
    glTranslatef(-94.9, 0, 3.45);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(.2, 14.6, 3.1);
    glTranslatef(-94.9, 0, 2.3);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glScalef(.2, 0.6, 1);
    glTranslatef(-94.8, 8.5, 9.3);
    cube(0, 0, 0);
    glPopMatrix();
}
void wall2()
{
    glPushMatrix();
    glScalef(3, 20, 4);
    glTranslatef(5.7, 0, -0.5);
    cube(1, 1, 1);
    glPopMatrix();

}
void floorWallsCeiling(float scale)
{
    //san nha
    glPushMatrix();
    glScalef(scale, 1, scale);
    glTranslatef(-0.5, -1, -0.5);
    cube(0.5, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(scale / 2, scale / 4, 0);
    glScalef(1, scale / 2, scale);
    glTranslatef(-2, -0.5, -0.5);
    cube(1, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-scale / 2 + 1, scale / 4, 0);
    glScalef(1, scale / 2, scale);
    glTranslatef(-1, -0.5, -0.5);
    cube(1, 1, 1);
    glPopMatrix();

    //mat bang
    glPushMatrix();
    glTranslatef(0, scale / 4, scale / 2);
    glScalef(scale, scale / 2, 1);
    glTranslatef(-0.5, -0.5, -1);
    cube(1, 1, 1);
    glPopMatrix();

    //back
    glPushMatrix();
    glTranslatef(0, scale / 4, scale / 2);
    glScalef(scale, scale / 2, 1);
    glTranslatef(-0.5, -0.5, -40);
    cube(1, 1, 1);
    glPopMatrix();

    //ceiling
    glPushMatrix();
    glTranslatef(0, scale / 2, 0);
    glScalef(scale, 1, scale);
    glTranslatef(-0.5, -1, -0.5);
    cube(1, 1, 0);
    glPopMatrix();
}

void light()
{
    GLfloat l_amb[] = { 0.2, 0.2, 0.2, 1.0 };// {đỏ, xanh lá,xanh biển, độ trong suốt}
    GLfloat l_dif[] = { 0.961, 0.871, 0.702 };
    GLfloat l_spec[] = { 0.2, 0.2, 0.2, 1 };
    GLfloat l_no[] = { 0, 0, 0, 1.0 };
    GLfloat l_pos1[] = { -20, 20, 20, 1.0 };
    GLfloat l_pos2[] = { 44, 30, -5, 1.0 };
    GLfloat l_pos3[] = { 0, 60, 0, 1.0 };

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    if (l_on1)
    {
        if (amb == true)
        {
            glLightfv(GL_LIGHT0, GL_AMBIENT, l_amb);
        }
        else
        {
            glLightfv(GL_LIGHT0, GL_AMBIENT, l_no);
        }

        if (dif == true)
        {
            glLightfv(GL_LIGHT0, GL_DIFFUSE, l_dif);
        }
        else
        {
            glLightfv(GL_LIGHT0, GL_DIFFUSE, l_no);
        }
        if (spec == true)
        {
            glLightfv(GL_LIGHT0, GL_SPECULAR, l_spec);
        }
        else
        {
            glLightfv(GL_LIGHT0, GL_SPECULAR, l_no);
        }
    }
    else
    {
        glLightfv(GL_LIGHT0, GL_AMBIENT, l_no);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, l_no);
        glLightfv(GL_LIGHT0, GL_SPECULAR, l_no);
    }
    glLightfv(GL_LIGHT0, GL_POSITION, l_pos1);

    if (l_on2)
    {
        if (amb == true)
        {
            glLightfv(GL_LIGHT1, GL_AMBIENT, l_amb);
        }
        else
        {
            glLightfv(GL_LIGHT1, GL_AMBIENT, l_no);
        }

        if (dif == true)
        {
            glLightfv(GL_LIGHT1, GL_DIFFUSE, l_dif);
        }
        else
        {
            glLightfv(GL_LIGHT1, GL_DIFFUSE, l_no);
        }
        if (spec == true)
        {
            glLightfv(GL_LIGHT1, GL_SPECULAR, l_spec);
        }
        else
        {
            glLightfv(GL_LIGHT1, GL_SPECULAR, l_no);
        }
    }
    else
    {
        glLightfv(GL_LIGHT1, GL_AMBIENT, l_no);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, l_no);
        glLightfv(GL_LIGHT1, GL_SPECULAR, l_no);
    }
    glLightfv(GL_LIGHT1, GL_POSITION, l_pos2);

    if (l_on3)
    {
        if (amb == true)
        {
            glLightfv(GL_LIGHT2, GL_AMBIENT, l_amb);
        }
        else
        {
            glLightfv(GL_LIGHT2, GL_AMBIENT, l_no);
        }

        if (dif == true)
        {
            glLightfv(GL_LIGHT2, GL_DIFFUSE, l_dif);
        }
        else
        {
            glLightfv(GL_LIGHT2, GL_DIFFUSE, l_no);
        }
        if (spec == true)
        {
            glLightfv(GL_LIGHT2, GL_SPECULAR, l_spec);
        }
        else
        {
            glLightfv(GL_LIGHT2, GL_SPECULAR, l_no);
        }
    }
    else
    {
        glLightfv(GL_LIGHT2, GL_AMBIENT, l_no);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, l_no);
        glLightfv(GL_LIGHT2, GL_SPECULAR, l_no);
    }

    glLightfv(GL_LIGHT2, GL_POSITION, l_pos3);

    GLfloat spot_direction[] = { 0.0, -1.0, 0.0 };
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);
    GLfloat spt_ct[] = { spt_cutoff };
    glLightfv(GL_LIGHT2, GL_SPOT_CUTOFF, spt_ct);
}

void CreateObjects()
{
    unsigned int cubeIndices[36];
    int idx = 0;

    for (int i = 0; i < 6; ++i) {
        cubeIndices[idx++] = quadIndices[i][0];
        cubeIndices[idx++] = quadIndices[i][1];
        cubeIndices[idx++] = quadIndices[i][2];
        cubeIndices[idx++] = quadIndices[i][2];
        cubeIndices[idx++] = quadIndices[i][3];
        cubeIndices[idx++] = quadIndices[i][0];
    }

    GLfloat cubeVertices[8 * 8]; // 8 vertices, 8 attributes per vertex (x, y, z, u, v, nx, ny, nz)
    int vertexIdx = 0;

    for (int i = 0; i < 8; ++i) {
        cubeVertices[vertexIdx++] = v_Cube[i][0];
        cubeVertices[vertexIdx++] = v_Cube[i][1];
        cubeVertices[vertexIdx++] = v_Cube[i][2];
        cubeVertices[vertexIdx++] = 0.0f; // u
        cubeVertices[vertexIdx++] = 0.0f; // v
        cubeVertices[vertexIdx++] = 0.0f; // nx
        cubeVertices[vertexIdx++] = 0.0f; // ny
        cubeVertices[vertexIdx++] = 0.0f; // nz
    }

    Mesh* cube = new Mesh();
    cube->CreateMesh(cubeVertices, cubeIndices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), sizeof(cubeIndices) / sizeof(cubeIndices[0]));
    meshList.push_back(cube);
}
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-3, 3, -3, 3, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (cameraFollowGhost) {
        GLfloat ghostCamX = ghostX - 2.0f * sin(glm::radians(ghostAngle));
        GLfloat ghostCamZ = ghostZ + 2.0f * cos(glm::radians(ghostAngle));
        GLfloat ghostCamY = ghostPositionY + 6.0f; // Điều chỉnh chiều cao của camera
        savedEyeGX = ghostX;
        savedEyeGY = ghostCamY ;
        savedEyeGZ = ghostZ;

        savedLookGX = ghostCamX;
        savedLookGY = ghostCamY;
        savedLookGZ = ghostCamZ;
        gluLookAt(savedEyeGX, savedEyeGY, savedEyeGZ, savedLookGX, savedLookGY, savedLookGZ, 0, 1, 0);
    }
    else {
        gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
    }


    glRotatef(rot, 0, 1, 0);
    light();
    fan_full_set();
    full_set_chair_table();
    floorWallsCeiling(40);
    wall2();
    window1();
    window2();
    board();
    bucgiang();
    ghostFull();
    projector();
    AC();
    ball();
    ball2();
    image();
    image2();
    projector_board();
    bulb_set();
    caugiaoFull();
    door();
    glFlush();
    glutSwapBuffers();
}

void myKeyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'e':
        rot++;
        break;
    case 'q':
        rot--;
        break;
    case 'd':
        if(cameraFollowGhost) {
            ghostAngle += 5.0f;
        }
        else {
            lookX++;
        }
        break;
    case 'a':
        if (cameraFollowGhost) {
            ghostAngle -= 5.0f; }
        else {
            lookX--;
        }
        break;
    case 's':
        if (cameraFollowGhost) {
            ghostX += 0.1f * sin(glm::radians(ghostAngle));
            ghostZ -= 0.1f * cos(glm::radians(ghostAngle));
        }
        else {
            eyeY++;
        }
        break;
    case 'w':
        if (cameraFollowGhost) {
            ghostX -= 0.1f * sin(glm::radians(ghostAngle));
            ghostZ += 0.1f * cos(glm::radians(ghostAngle));
        }
        else {
            eyeY--;
        }
        break;
    case 'z':
        if (!cameraFollowGhost) {
            eyeX = 0;
            eyeZ++;
            lookZ++;
        }
        break;
    case 'x':
        if (!cameraFollowGhost) {
            eyeZ--;
        }
        break;
    case '1':
        l_on1 = !l_on1;
        break;
    case '2':
        l_on2 = !l_on2;
        break;
    case '3':
        l_on3 = !l_on3;
        break;
    case '4':
        amb = !amb;
        break;
    case '5':
        spec = !spec;
        break;
    case '6':
        dif = !dif;
        break;
    case 'p':
        isMovingUp = !isMovingUp;
        break;
    case 'r':
        fanRotate = !fanRotate;// Toggle fan rotation
        if (fanRotate) {
            off = 0;
            on = 1;
        }
        else { off = 1; on = 0; }
        break;
    case 'c':
        cameraFollowGhost = !cameraFollowGhost;
        if (cameraFollowGhost) {
            // Lưu vị trí và góc nhìn của camera hiện tại
            savedEyeX = eyeX;
            savedEyeY = eyeY;
            savedEyeZ = eyeZ;
            savedLookX = lookX;
            savedLookY = lookY;
            savedLookZ = lookZ;

            
            // Đặt lại camera về vị trí và góc nhìn của ghost
            eyeX = ghostX;
            eyeY = ghostPositionY + 1.0f; // Điều chỉnh chiều cao của camera
            eyeZ = ghostZ;
            lookX = ghostX + 2.0f * sin(glm::radians(ghostAngle));
            lookY = eyeY;
            lookZ = ghostZ - 2.0f * cos(glm::radians(ghostAngle));
        }
        else {
            // Khôi phục vị trí và góc nhìn của camera
            eyeX = savedEyeX;
            eyeY = savedEyeY;
            eyeZ = savedEyeZ;
            lookX = savedLookX;
            lookY = savedLookY;
            lookZ = savedLookZ;
        }
        break;
    case 'i':
        showSquare = !showSquare; // Toggle square animation
        if (!showSquare) {
            squareSize = 0.0f; // Reset square size when turning off animation
        }
        break;
    }
}
void myMouseMotionFunc(int x, int y)
{

    static int lastX = -1, lastY = -1;

    if (lastX == -1 || lastY == -1)
    {
        lastX = x;
        lastY = y;
    }

    int deltaX = x - lastX;
    int deltaY = y - lastY;

    lookX += deltaX * 0.1f;
    lookY -= deltaY * 0.1f;

    lastX = x;
    lastY = y;

    // Ensure the lookY does not go beyond vertical limits
    if (lookY > 89.0f) lookY = 89.0f;
    if (lookY < -89.0f) lookY = -89.0f;
}

void animate()
{
    //fan:
    if (fanRotate) // Only rotate the fan if fanRotate is true
    {
        alpha += 0.5;
        if (alpha > 360) alpha -= 360 * floor(alpha / 360);
    }
    //proboard:
    if (isMovingUp) {
        boardPositionY -= movementSpeed;
    }
    else {
        boardPositionY += movementSpeed;
    }
    // Giới hạn phạm vi di chuyển của bảng
    if (boardPositionY > 0.3f) boardPositionY = 0.3f;
    if (boardPositionY < -0.5f) boardPositionY = -0.5f;

    //ghost
    // Update ghost's position
    if (isFloat) {
        ghostPositionY -= movementSpeed1;
    }
    else {
        ghostPositionY += movementSpeed1;
    }
    // Switch direction when reaching the bounds
    if (ghostPositionY > 0.3f) {
        ghostPositionY = 0.3f;
        isFloat = true; // Switch to moving down
    }
    if (ghostPositionY < -0.5f) {
        ghostPositionY = -0.5f;
        isFloat = false; // Switch to moving up
    }

    if (showSquare) {
        // Increase square size for animation effect
        squareSize += 0.02f;
        if (squareSize > 1.0f) {
            squareSize = 8.0f; // Clamp square size to 1.0
        }
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // Khởi tạo cửa sổ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("ClassRoom 04180201252");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Error initializing GLEW\n");
        return -1;
    }

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);

    // Tạo đối tượng và shader
    CreateObjects();
    CreateShaders();

    // Thiết lập camera
    camera = Camera(glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 5.0f, 0.2f);

    // Đăng ký callback
    glutKeyboardFunc(myKeyboardFunc);
    glutDisplayFunc(display);
    glutIdleFunc(animate);
    glutMotionFunc(myMouseMotionFunc);
    textureID = loadTexture("Textures/WM.jpg"); 
    textureID1 = loadTexture("Textures/thayvan.png");
    textureID2 = loadTexture("Textures/cei.jpg");
    glutMainLoop();

    return 0;
}