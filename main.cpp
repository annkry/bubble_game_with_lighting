#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <iostream>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>

GLFWwindow *window;
#include "glm/ext.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
#include <shader.hpp>
#include <algorithm>

GLfloat g_data[] = {
            // UP
            0.0f, 1.0f, 2.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            // UP
            0.0f, 1.0f, 2.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 2.0f,
            // DOWN
            0.0f, 0.0f, 2.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            // DOWN
            0.0f, 0.0f, 2.0f,
            1.0f, 0.0f, 2.0f,
            1.0f, 0.0f, 0.0f,
            // LRR
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 2.0f,
            1.0f, 1.0f, 2.0f,
            // LRR
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 2.0f,
            1.0f, 1.0f, 0.0f,
            // LRL
            0.0f, 1.0f, 2.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            // LRL
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 2.0f,
            0.0f, 0.0f, 2.0f,
            // RLR
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            // RLR
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            // RLL
            1.0f, 1.0f, 2.0f,
            1.0f, 0.0f, 2.0f,
            0.0f, 0.0f, 2.0f,
            // RLL
            0.0f, 1.0f, 2.0f,
            1.0f, 1.0f, 2.0f,
            0.0f, 0.0f, 2.0f};

glm::vec3 camUp = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 camPos;
glm::mat4 VPawn;

float PawnX;
float PawnY;
float PawnZ;

float PawnDirX;
float PawnDirY;
float PawnDirZ;

float radiusPawn;

bool ifPawnView;

int h;
int w;
int N;

bool finish;
bool secondLevel;
GLfloat pawnSpeed;

float finishTime;
float startTime;

int partCount = 15;
int fragmentCount = 10;

float prevMouseX;
float prevMouseY;

#define space 10
#define vertexcount partCount * fragmentCount * 3 * 3

const float PI = 3.1415926535897;

glm::vec3 sphere[40 * 20 * 3 * 3];
std::vector<glm::vec3> ball_vert[40 * 20 * 3 * 3];
GLfloat ballVertForPawn[2520];

glm::vec3 obs_vert[50];

float radiusMovingObs;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::vec3 position;
glm::vec3 position1;
glm::vec2 centralPoint;
int points;
bool lightMode;

float horizontalAngle = 0.55f * 3.1415926535897f;

float verticalAngle = 0.2f * 3.1415926535897f;

float initialFoV = 45.0f;
float speed = 2.0f;
float mouseSpeed = 0.005f;

int numberOfObs = 20;
int maxObs = 50;
GLfloat g_vert_data[40 * 40 * 3 * 3 * 50];
GLfloat g_position_data[50 * 4];
GLfloat g_color_data[50 * 3];
GLfloat g_axis_data[50 * 3];
GLfloat g_angles_data[50 * 2];
GLfloat g_normal_data[40 * 40 * 3 * 3 * 50];

GLfloat g_game_field_color_data[12 * 3 * 3];
glm::vec3 game_field_color_container[12 * 3];


glm::vec3 cameraPosition;
std::vector<glm::vec3> vertForView[40 * 20 * 3 * 3];

glm::vec3 lights[20];
glm::vec3 Obslights[20];

float maxVelocityForSpheres;

struct BallObs
{
    glm::vec3 pos;
    float speed;
    float r, g, b;
    float radius;
    std::vector<glm::vec3> vert[40 * 20 * 3 * 3];
    glm::vec3 axis;
    bool blink;
    bool draw;

    bool operator<(const BallObs &that) const
    {
        return length(this->pos - cameraPosition) > length(that.pos - cameraPosition);
    }
};

std::vector<std::unique_ptr<BallObs>> obsTable;

float alpha;

glm::vec2 generateRandomBallCoord(float minim, float maxim)
{
    return vec2(minim + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxim - minim))), minim + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxim - minim))));
}

int check(float newX, float newY, float newZ)
{
    if (newZ > (2.0 - radiusPawn))
    {
        if(secondLevel){
            finish = true;
        }
        else{
            secondLevel = true;
        }
        
    }
    // checks whether our pawn has not left the cube
    if ((newX > (1.0 - radiusPawn)) || (newX < radiusPawn) || (newY > (1.0 - radiusPawn)) || (newY < radiusPawn) || (newZ > (2.0 - radiusPawn)) || (newZ < radiusPawn))
    {
        return 1;
    }
    return 2;
}

void computeMatricesFromInputs(float near)
{
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int checkIfValid;
    horizontalAngle += mouseSpeed * (centralPoint.x - xpos);
    verticalAngle += mouseSpeed * (centralPoint.y - ypos);
    glm::vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        verticalAngle -= mouseSpeed * 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        verticalAngle += mouseSpeed * 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        horizontalAngle += mouseSpeed * 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        horizontalAngle -= mouseSpeed * 3.0f;
    }
    centralPoint = vec2(xpos, ypos);
    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f));
    glm::vec3 camMoveDir = glm::vec3(PawnDirX - PawnX, PawnDirY - PawnY, PawnDirZ - PawnZ);
    camMoveDir = normalize(camMoveDir);
    glm::vec3 move = camMoveDir * 0.01f;
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        if(initialFoV > 3.0)
            initialFoV -= 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    {
        if(initialFoV <= 97.0)
            initialFoV += 3.0f;
    }
    position1.x = (position.x + direction.x) * 3.0f;
    position1.y = (position.y + direction.y) * 3.0f;
    position1.z = (position.z + direction.z) * 3.0f;

    PawnDirX = position1.x;
    PawnDirY = position1.y;
    PawnDirZ = position1.z;

    float FoV = initialFoV; 
    ProjectionMatrix = glm::perspective(glm::radians(FoV), (GLfloat)w / (GLfloat)h, near, 100.0f);

    if (!ifPawnView)
    {
        position1 = vec3(0.5, 0.5, 1.0);
        cameraPosition = vec3(-1.9, 0.5, 1.0);
        ViewMatrix = glm::lookAt(
            vec3(-1.9, 0.5, 1.0),
            position1,
            camUp
        );
    }
    else
    {
        cameraPosition = camPos;
        ViewMatrix = glm::lookAt(
            camPos,
            position1,
            camUp
        );
        VPawn = ViewMatrix;
    }
    lastTime = currentTime;
}

void createsphere(float radius, std::vector<glm::vec3> sphere_vert[])
{
    int idx = 0;
    float x, y, z, g;
    float fragmentStep = 2 * PI / fragmentCount;
    float partStep = PI / partCount;
    float sectorAngle, partAngle;
    glm::vec3 sphereNormal[40 * 20 * 3 * 3];
    float radInv = 1.0f / radius;

    for (int i = 0; i <= partCount; ++i)
    {
        partAngle = PI / 2 - i * partStep;
        g = radius * cosf(partAngle);
        z = radius * sinf(partAngle);
        for (int j = 0; j <= fragmentCount; ++j)
        {
            sectorAngle = j * fragmentStep;
            x = g * cosf(sectorAngle);
            y = g * sinf(sectorAngle);
            sphere[idx] = glm::vec3(x, y, z);
            sphereNormal[idx] = glm::vec3(x * radInv, y * radInv, z * radInv);
            idx += 1;
        }
    }
    int frag, part;
    idx = 0;
    for (int i = 0; i < partCount; ++i)
    {
        frag = i * (fragmentCount + 1);
        part = frag + fragmentCount + 1;
        for (int j = 0; j < fragmentCount; ++j, ++frag, ++part)
        {
            if (i != 0)
            {
                sphere_vert[idx].push_back(glm::vec3(sphere[frag].x, sphere[frag].y, sphere[frag].z));
                sphere_vert[idx].push_back(glm::vec3(sphere[part].x, sphere[part].y, sphere[part].z));
                sphere_vert[idx].push_back(glm::vec3(sphere[frag + 1].x, sphere[frag + 1].y, sphere[frag + 1].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[frag].x, sphereNormal[frag].y, sphereNormal[frag].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[part].x, sphereNormal[part].y, sphereNormal[part].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[frag + 1].x, sphereNormal[frag + 1].y, sphereNormal[frag + 1].z));
                idx += 1;
            }
            if (i != (partCount - 1))
            {
                sphere_vert[idx].push_back(glm::vec3(sphere[frag + 1].x, sphere[frag + 1].y, sphere[frag + 1].z));
                sphere_vert[idx].push_back(glm::vec3(sphere[part].x, sphere[part].y, sphere[part].z));
                sphere_vert[idx].push_back(glm::vec3(sphere[part + 1].x, sphere[part + 1].y, sphere[part + 1].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[frag + 1].x, sphereNormal[frag + 1].y, sphereNormal[frag + 1].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[part].x, sphereNormal[part].y, sphereNormal[part].z));
                sphere_vert[idx].push_back(glm::vec3(sphereNormal[part + 1].x, sphereNormal[part + 1].y, sphereNormal[part + 1].z));
                idx += 1;
            }
        }
    }
}

class Ball
{
public:
    Ball(float r)
    {
        radius = r;
        programID = LoadShaders("PawnVS.vertexshader", "PawnFS.fragmentshader");
        bufferId = setBuffers();
        MatrixID = glGetUniformLocation(programID, "MVP");
    }

    GLuint setBuffers()
    {
        GLuint particles_position_buffer;
        glGenVertexArrays(1, &VertexArrayID);
        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        return particles_position_buffer;
    }

    void draw(float near, glm::vec3 cP, glm::vec3 cD, bool ifView)
    {
        glBindVertexArray(VertexArrayID);
        glUseProgram(programID);
        computeMatricesFromInputs(near);
        glm::mat4 Projection = ProjectionMatrix;
        glm::mat4 View = ViewMatrix;

        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ARRAY_BUFFER, 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2520, ballVertForPawn);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::translate(Model, vec3(PawnX, PawnY, PawnZ));
        glm::mat4 MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        if (ifView)
            glUniform1f(4, 1.0);
        else
            glUniform1f(4, 0.0);

        glVertexAttribDivisor(0, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 2520, 1);
    }

private:
    float radius;
    GLuint programID;
    GLuint bufferId;
    GLuint MatrixID;
    GLuint VertexArrayID;
};

class Cube
{
public:
    Cube()
    {
        std::pair<GLuint, GLuint> result = setBuffers();
        bufferId = result.first;
        colorBufferId = result.second;
        programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
        MatrixID = glGetUniformLocation(programID, "MVP");
    }

    std::pair<GLuint, GLuint> setBuffers()
    {
        //data filling
        for(int i = 0; i < 6; i++){
            game_field_color_container[i] = vec3(0.45f, 0.61f, 0.76f);
        }

        for(int i = 6; i < 12; i++){
            game_field_color_container[i] = vec3(0.54f, 0.78f, 0.93f);
        }

        for(int i = 12; i < 24; i++){
            game_field_color_container[i] = vec3(0.4f, 0.67f, 0.85f);
        }

        for(int i = 24; i < 36; i++){
            game_field_color_container[i] = vec3(0.46f, 0.76f, 0.97f);
        }

        GLuint particles_position_buffer;
        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, 3 * 12 * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        GLuint particles_color_buffer;
        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER,  3 * 12 * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        glGenVertexArrays(1, &VertexArrayID);
        return std::make_pair(particles_position_buffer, particles_color_buffer);
    }

    void draw(float near, glm::vec3 cP, glm::vec3 cD)
    {
        glBindVertexArray(VertexArrayID);
        glUseProgram(programID);

        computeMatricesFromInputs(near);
        glm::mat4 Projection = ProjectionMatrix;
        glm::mat4 View = ViewMatrix;

        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        //data filling
        int id = 0;
        if (lightMode){
            for(int i = 0; i < 6; i++){
                g_game_field_color_data[id + 0] = 0.14f;
                g_game_field_color_data[id + 1] = 0.17f;
                g_game_field_color_data[id + 2] = 0.77f;
                id += 3;
            }
            for(int i = 6; i < 12; i++){
                g_game_field_color_data[id + 0] = 0.93f;
                g_game_field_color_data[id + 1] = 0.88f;
                g_game_field_color_data[id + 2] = 0.23f;
                id += 3;
            }
        }
        else{
            for(int i = 0; i < 6; i++){
                g_game_field_color_data[id + 0] = game_field_color_container[i].x;
                g_game_field_color_data[id + 1] = game_field_color_container[i].y;
                g_game_field_color_data[id + 2] = game_field_color_container[i].z;
                id += 3;
            }
            for(int i = 6; i < 12; i++){
                g_game_field_color_data[id + 0] = game_field_color_container[i].x;
                g_game_field_color_data[id + 1] = game_field_color_container[i].y;
                g_game_field_color_data[id + 2] = game_field_color_container[i].z;
                id += 3;
            }
        }

        for(int i = 12; i < 36; i++){
            g_game_field_color_data[id + 0] = game_field_color_container[i].x;
            g_game_field_color_data[id + 1] = game_field_color_container[i].y;
            g_game_field_color_data[id + 2] = game_field_color_container[i].z;
            id += 3;
        }

        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ARRAY_BUFFER, 12 * 3 * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * 3 * 3 * sizeof(GLfloat), g_data);

        glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);
        glBufferData(GL_ARRAY_BUFFER, 12 * 3 * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * 3 * 3 * sizeof(GLfloat), g_game_field_color_data);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferId);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glUniform3f(2, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        if (ifPawnView)
            glUniform1f(11, 1.0);
        else
            glUniform1f(11, 0.0);

        if (lightMode)
            glUniform1f(4, 1.0);
        else
            glUniform1f(4, 0.0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 12 * 3, 1);
    }

private:
    GLuint programID;
    GLuint bufferId;
    GLuint colorBufferId;
    GLuint VertexArrayID;
    GLuint MatrixID;
};

bool compareDistView(std::vector<glm::vec3> a, std::vector<glm::vec3> b)
{
    glm::vec3 p1;
    glm::vec3 p2;
    p1 = a[0] + vec3(0.2 * (a[1] - a[0]).x, 0.2 * (a[2] - a[0]).y, 0.2 * (a[2] - a[0]).z) + vec3(0.2 * (a[2] - a[0]).x, 0.2 * (a[2] - a[0]).y, 0.2 * (a[2] - a[0]).z);
    p2 = b[0] + vec3(0.2 * (b[1] - b[0]).x, 0.2 * (b[2] - b[0]).y, 0.2 * (b[2] - b[0]).z) + vec3(0.2 * (b[2] - b[0]).x, 0.2 * (b[2] - b[0]).y, 0.2 * (b[2] - b[0]).z);
    return length(vec3(-1.9, 0.5, 1.0) - p1) > length(vec3(-1.9, 0.5, 1.0) - p2);
}

void SortParticlesZ()
{
    std::sort(obsTable.begin(), obsTable.end());
}

void SortParticleVertView()
{
    std::sort(&vertForView[0], &vertForView[280], compareDistView);
}

class MovingObstacles
{
public:
    MovingObstacles()
    {
        programID = LoadShaders("MovingObsVS.vertexshader", "MovingObsFS.fragmentshader");
        std::vector<GLuint> result = setBuffers();
        bufferVertId = result[0];
        bufferPosId = result[1];
        bufferColId = result[2];
        bufferNormalId = result[3];
        bufferAxisId = result[4];
        bufferAnglesId = result[5];
        MatrixID = glGetUniformLocation(programID, "VP");
        for (int i = 0; i < numberOfObs; i++)
        {
            auto newBall = std::make_unique<BallObs>();
            newBall->radius = generateRandomBallCoord(0.01, 0.12).x;
            newBall->pos = vec3(generateRandomBallCoord(newBall->radius + 0.1, 1.0 - newBall->radius).x, newBall->radius, generateRandomBallCoord(newBall->radius, 2.0 - newBall->radius).y);
            newBall->speed = generateRandomBallCoord(0.0, maxVelocityForSpheres).x;
            newBall->r = generateRandomBallCoord(0.0, 1.0).x;
            newBall->g = generateRandomBallCoord(0.0, 1.0).y;
            newBall->b = generateRandomBallCoord(0.0, 1.0).x;
            newBall->draw = true;
            if(i%2 == 0){
                newBall->blink = true;
            }
            createsphere(newBall->radius, newBall->vert);
            obsTable.push_back(std::move(newBall));
        }
        VID = glGetUniformLocation(programID, "V");
        PID = glGetUniformLocation(programID, "P");
        VPawnID = glGetUniformLocation(programID, "VPawn");
        LightID = glGetUniformLocation(programID, "lightTable");
    }

    bool crushWithPawn()
    {
        bool result = false;
        for (int i = 0; i < numberOfObs; i++)
        {
            float dist = 
                sqrt((PawnX - obsTable[i]->pos.x) *
                     (PawnX - obsTable[i]->pos.x) +
                     (PawnY - obsTable[i]->pos.y) *
                     (PawnY - obsTable[i]->pos.y) +
                     (PawnZ - obsTable[i]->pos.z) *
                     (PawnZ - obsTable[i]->pos.z));
            if (obsTable[i]->draw && dist <= obsTable[i]->radius + radiusPawn){
                if(obsTable[i]->blink){
                    points += 3;
                    obsTable[i]->draw = false;
                    result = false;
                }
                else{
                    result = true;
                }
            }
        }
        return result;
    }

    void position(int idObs) // Moving obstalce is making a move forward
    {
        obsTable[idObs]->pos.y += obsTable[idObs]->speed;
        if (obsTable[idObs]->radius < 0.13 && obsTable[idObs]->pos.x - obsTable[idObs]->radius - obsTable[idObs]->pos.y * obsTable[idObs]->radius * 0.005 > 0.1)
            obsTable[idObs]->radius += obsTable[idObs]->pos.y * obsTable[idObs]->radius * 0.005;
        if (1.0 - obsTable[idObs]->pos.y <= obsTable[idObs]->radius)
        {
            obsTable[idObs]->radius = generateRandomBallCoord(0.01, 0.12).x;
            obsTable[idObs]->pos = vec3(generateRandomBallCoord(0.1 + obsTable[idObs]->radius, 1.0 - obsTable[idObs]->radius).x, obsTable[idObs]->radius, generateRandomBallCoord(0.0 + obsTable[idObs]->radius, 2.0 - obsTable[idObs]->radius).y);
    
            obsTable[idObs]->speed = generateRandomBallCoord(0.0, maxVelocityForSpheres).x;

            obsTable[idObs]->r = generateRandomBallCoord(0.0, 1.0).x;
            obsTable[idObs]->g = generateRandomBallCoord(0.0, 1.0).y;
            obsTable[idObs]->b = generateRandomBallCoord(0.0, 1.0).x;
            obsTable[idObs]->draw = true;
        }
    }

    std::vector<GLuint> setBuffers()
    {
        std::vector<GLuint> res;
        GLuint particles_position_buffer;
        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        res.push_back(particles_position_buffer);

        GLuint particles_color_buffer;
        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        GLuint bufferc;
        glGenBuffers(1, &bufferc);
        glBindBuffer(GL_ARRAY_BUFFER, bufferc);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        GLuint normalbuffer;
        glGenBuffers(1, &normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        GLuint bufferaxis;
        glGenBuffers(1, &bufferaxis);
        glBindBuffer(GL_ARRAY_BUFFER, bufferaxis);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        GLuint bufferangles;
        glGenBuffers(1, &bufferangles);
        glBindBuffer(GL_ARRAY_BUFFER, bufferangles);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        glGenVertexArrays(1, &VertexArrayID);

        res.push_back(particles_color_buffer);
        res.push_back(bufferc);
        res.push_back(normalbuffer);
        res.push_back(bufferaxis);
        res.push_back(bufferangles);
        return res;
    }

    void draw(float prevTime, float currTime, float near, glm::vec3 cP, glm::vec3 cD)
    {
        glBindVertexArray(VertexArrayID);
        glUseProgram(programID);
        computeMatricesFromInputs(near);
        glm::mat4 Projection = ProjectionMatrix;
        glm::mat4 View = ViewMatrix;

        if (currTime - prevTime >= 0.2)
        {
            for (int i = 0; i < numberOfObs; i++)
            {
                position(i);
            }
        }

        //sorting by a ball position
        SortParticlesZ();

        //data filling
        int pom = 0;
        for (int j = 0; j < 2520; j += 9)
        {

            g_vert_data[j + 0] = vertForView[pom][0].x;
            g_vert_data[j + 1] = vertForView[pom][0].y;
            g_vert_data[j + 2] = vertForView[pom][0].z;
            g_vert_data[j + 3] = vertForView[pom][1].x;
            g_vert_data[j + 4] = vertForView[pom][1].y;
            g_vert_data[j + 5] = vertForView[pom][1].z;
            g_vert_data[j + 6] = vertForView[pom][2].x;
            g_vert_data[j + 7] = vertForView[pom][2].y;
            g_vert_data[j + 8] = vertForView[pom][2].z;
            g_normal_data[j + 0] = vertForView[pom][3].x;
            g_normal_data[j + 1] = vertForView[pom][3].y;
            g_normal_data[j + 2] = vertForView[pom][3].z;
            g_normal_data[j + 3] = vertForView[pom][4].x;
            g_normal_data[j + 4] = vertForView[pom][4].y;
            g_normal_data[j + 5] = vertForView[pom][4].z;
            g_normal_data[j + 6] = vertForView[pom][5].x;
            g_normal_data[j + 7] = vertForView[pom][5].y;
            g_normal_data[j + 8] = vertForView[pom][5].z;
            pom += 1;
        }

        int howManyToDraw = numberOfObs;
        
        int i = 0, j = 0;
        while(i < howManyToDraw)
        {

            BallObs& p = *obsTable[j++];
            if(p.draw){
                g_position_data[4 * i + 0] = p.pos.x;
                g_position_data[4 * i + 1] = p.pos.y;
                g_position_data[4 * i + 2] = p.pos.z;
                g_position_data[4 * i + 3] = p.radius;

                g_color_data[3 * i + 0] = p.r;
                g_color_data[3 * i + 1] = p.g;
                g_color_data[3 * i + 2] = p.b;

                if(cameraPosition.z > p.pos.z){
                    g_angles_data[2 * i + 0] = acos(glm::dot(vec3(-1.0, 0.0, 0.0), normalize(vec3(cameraPosition.x - p.pos.x, 0.0, cameraPosition.z - p.pos.z))));
                }
                else{
                    g_angles_data[2 * i + 0] = -acos(glm::dot(vec3(-1.0, 0.0, 0.0), normalize(vec3(cameraPosition.x - p.pos.x, 0.0, cameraPosition.z - p.pos.z))));
                }
                

                if(cameraPosition.y - p.pos.y > 0){
                    g_angles_data[2 * i + 1] = -asin(abs(cameraPosition.y - p.pos.y) / length(vec3(p.pos.x - cameraPosition.x, p.pos.y- cameraPosition.y, p.pos.z - cameraPosition.z)));
                    
                }
                else{
                    g_angles_data[2 * i + 1] = asin(abs(cameraPosition.y - p.pos.y) / length(vec3(p.pos.x - cameraPosition.x, p.pos.y - cameraPosition.y, p.pos.z- cameraPosition.z)));
                }
                vec3 cross = glm::cross(normalize(vec3(p.pos.x - cameraPosition.x, p.pos.y - cameraPosition.y, p.pos.z - cameraPosition.z)), vec3(0.0, 1.0, 0.0));
                
                g_axis_data[3 * i + 0] = cross.x;
                g_axis_data[3 * i + 1] = cross.y;
                g_axis_data[3 * i + 2] = cross.z;
                i +=1;
            }
            else{
                howManyToDraw -= 1;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, bufferVertId);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2520, g_vert_data);

        glBindBuffer(GL_ARRAY_BUFFER, bufferPosId);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, numberOfObs * sizeof(GLfloat) * 4, g_position_data);

        glBindBuffer(GL_ARRAY_BUFFER, bufferColId);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, numberOfObs * sizeof(GLfloat) * 3, g_color_data);

        glBindBuffer(GL_ARRAY_BUFFER, bufferNormalId);
        glBufferData(GL_ARRAY_BUFFER, 2520 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2520, g_normal_data);

        glBindBuffer(GL_ARRAY_BUFFER, bufferAnglesId);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); 
        glBufferSubData(GL_ARRAY_BUFFER, 0, numberOfObs * sizeof(GLfloat) * 2, g_angles_data);

        glBindBuffer(GL_ARRAY_BUFFER, bufferAxisId);
        glBufferData(GL_ARRAY_BUFFER, maxObs * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numberOfObs * sizeof(GLfloat) * 3, g_axis_data);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferVertId);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, bufferPosId);
        glVertexAttribPointer(
            1,
            4,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, bufferColId);
        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, bufferNormalId);
        glVertexAttribPointer(
            3,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, bufferAnglesId);
        glVertexAttribPointer(
            4,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glEnableVertexAttribArray(5);
        glBindBuffer(GL_ARRAY_BUFFER, bufferAxisId);
        glVertexAttribPointer(
            5,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 VP = Projection * View;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &VP[0][0]);
        glUniformMatrix4fv(PID, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(VID, 1, GL_FALSE, &View[0][0]);
        glUniform3fv(LightID, 10, glm::value_ptr(lights[0]));

        if(lightMode)
            glUniform1f(11, 1.0);
        else
            glUniform1f(11, 0.0);

        int id=0;
        for(int i = 0; i < numberOfObs; i++){
            if(obsTable[i]->blink){
                Obslights[id + 0] = obsTable[i]->pos;
                Obslights[id + 1] = glm::vec3(obsTable[i]->r, obsTable[i]->g, obsTable[i]->b);
                id += 2;
            }
        }

        glUniform3f(10, cameraPosition.x, cameraPosition.y, cameraPosition.z);
        glUniform3f(47, PawnX, PawnY, PawnZ);
        glUniform3fv(34, 10, glm::value_ptr(Obslights[0]));
        glUniform1f(6, alpha);

        if (ifPawnView)
            glUniform1f(46, 1.0);
        else
            glUniform1f(46, 0.0);

        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 0);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 2520, howManyToDraw);
    }

private:
    Ball ball = Ball(radiusMovingObs);
    GLuint programID;
    GLuint bufferVertId;
    GLuint bufferPosId;
    GLuint bufferColId;
    GLuint bufferAnglesId;
    GLuint bufferAxisId;
    GLuint MatrixID;
    GLuint LightID;
    GLuint VertexArrayID;
    GLuint bufferNormalId;
    GLuint PID;
    GLuint VID;
    GLuint VPawnID;
};

void init()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    h = 600;
    w = 800;

    PawnX = 0.5;
    PawnY = 0.5;
    PawnZ = 1.0 / (2 * N);
    camPos = vec3(PawnX, PawnY, PawnZ);
    pawnSpeed = 0.01f;

    PawnDirX = 0.5;
    PawnDirY = 0.5;
    PawnDirZ = 0.5;
    ifPawnView = true;
    secondLevel = false;
    points = 0;
    lightMode = false;
    maxVelocityForSpheres = 0.022;


    position = vec3(PawnX, PawnY, PawnZ);
    centralPoint = vec2((float)w / 4.0, (float)h / 2.0);
    position1 = vec3(0.5, 0.5, 0.5);

    window = glfwCreateWindow(800, 600, "Bubble game", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
    }
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.51f, 0.87f, 0.9f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    startTime = glfwGetTime();

    for (unsigned int i = 0; i < numberOfObs; i++)
    {
        obs_vert[i] = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
    }

    radiusPawn = 1.0 / (10 * N);
    prevMouseY = -1.0;
    createsphere(radiusPawn, ball_vert); 
    alpha = 1.0;
    int id = 0;

    for (int i = 0; i < 280; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            vertForView[i].push_back(ball_vert[i][j]);
            ballVertForPawn[id + 0] = ball_vert[i][j].x;
            ballVertForPawn[id + 1] = ball_vert[i][j].y;
            ballVertForPawn[id + 2] = ball_vert[i][j].z;
            id += 3;
        }
        for (int j = 3; j < 6; j++)
        {
            vertForView[i].push_back(ball_vert[i][j]);
        }
    }

    //sorting all triangles on a sphere
    SortParticleVertView();

    //generate random lights from a floor and a ceiling
    for(int i = 0; i < 10; i++){
        lights[i] = vec3(generateRandomBallCoord(0.0, 1.0).x, -1.0, generateRandomBallCoord(0.5, 1.0).x);
    }
    for(int i = 10; i < 20; i++){
        lights[i] = vec3(generateRandomBallCoord(0.0, 1.0).x, 2.0, generateRandomBallCoord(0.5, 1.0).x);
    }
}

void initLevel2()
{
    numberOfObs = 28;
    pawnSpeed = 0.01f;
    PawnX = 0.5; 
    PawnY = 0.5; 
    PawnZ = 1.0 / (2 * N);
    camPos = vec3(PawnX, PawnY, PawnZ);
    horizontalAngle = 0.55f * 3.1415926535897f;
    startTime = glfwGetTime();
    verticalAngle = 0.2f * 3.1415926535897f;
    prevMouseY = -1.0;
    prevMouseY = -1.0;

    PawnDirX = 0.5;
    PawnDirY = 0.5;
    PawnDirZ = 0.5;
    ifPawnView = true;
    points = 0;
    lightMode = false;
    maxVelocityForSpheres = 0.036;

    position = vec3(PawnX, PawnY, PawnZ);
    centralPoint = vec2((float)w / 4.0, (float)h / 2.0);
    position1 = vec3(0.5, 0.5, 0.5);

    for (int i = 0; i < obsTable.size() - numberOfObs; i++)
    {
        auto newBall = std::make_unique<BallObs>();
        newBall->radius = generateRandomBallCoord(0.01, 0.12).x;
        newBall->pos = vec3(generateRandomBallCoord(newBall->radius + 0.1, 1.0 - newBall->radius).x, 0.0f, generateRandomBallCoord(newBall->radius, 2.0 - newBall->radius).y);
        newBall->speed = generateRandomBallCoord(0.0, 0.01).x;
        newBall->r = generateRandomBallCoord(0.0, 1.0).x;
        newBall->g = generateRandomBallCoord(0.0, 1.0).y;
        newBall->b = generateRandomBallCoord(0.0, 1.0).x;   
        newBall->draw = true;

        for (int i = 0; i < 280; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                newBall->vert[i].push_back(ball_vert[i][j]);
            }
        }
        obsTable.push_back(std::move(newBall));
    }
}

int main(int argc, char *argv[])
{
    N = 10;
    finish = false;
    vec3 lookAt;
    vec3 cameraPosition;
    init();

    Ball pawn = Ball(radiusPawn);
    Cube cube = Cube();
    MovingObstacles movingObstacles = MovingObstacles();

    float near = radiusPawn;
    int height;
    int width;
    float currMouseX;
    float currMouseY;
    float prevTime = glfwGetTime();
    float currTime;
    float currMoveTime;
    float prevMoveTime = glfwGetTime();
    int checkIfValid;
    glm::vec3 direction;
    glm::vec3 position;
    bool printCommunicate = true;
    float newX;
    float newY;
    float newZ;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetWindowSize(window, &width, &height);
        w = width;
        h = height;
        glViewport(0, 0, w, h);
        lookAt = glm::vec3(PawnDirX, PawnDirY, PawnDirZ);
        cameraPosition = glm::vec3(PawnX, PawnY, PawnZ);

        //drawing elements via instancing
        pawn.draw(near, cameraPosition, lookAt, false);
        cube.draw(near, cameraPosition, lookAt);
        movingObstacles.draw(prevTime, currTime, near, cameraPosition, lookAt);

        currTime = glfwGetTime();

        //end of first level
        if (secondLevel && printCommunicate)
        { 
            float finishTime = glfwGetTime();
            std::cout<<"Your final points for first level: "<<PawnZ / (finishTime - startTime) * 20 / 0.14814815 + points<<"\n";
            initLevel2();
            printCommunicate = false;
        }

        //end of second level
        if(finish){
            float finishTime = glfwGetTime();
            std::cout<<"Your final points for second level: "<<PawnZ / (finishTime - startTime) * 20 / 0.14814815 + points<<"\n";
            break;
        }

        //view switching
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE){
                ifPawnView = !ifPawnView;
                if (!ifPawnView)
                {
                    near = 1.95;
                }
                else
                {
                    near = radiusPawn;
                }
            }
        }

        //turning on and off extra light mode
        if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS){
            lightMode = true;
        }
        if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
            lightMode = false;
        }

        //moving forward or backward
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            direction = glm::vec3(PawnDirX - PawnX, PawnDirY - PawnY, PawnDirZ - PawnZ);
            direction = normalize(direction);
            currMoveTime = glfwGetTime();
            position = direction * pawnSpeed;
            newX = PawnX + position.x;
            newY = PawnY + position.y;
            newZ = PawnZ + position.z;
            checkIfValid = check(newX, newY, newZ);
            if (checkIfValid == 2)
            {
                if ((currMoveTime - prevMoveTime) > 0.001)
                {
                    prevMoveTime = currMoveTime;

                    PawnX = newX;
                    PawnY = newY;
                    PawnZ = newZ;
                    camPos += position;

                    PawnDirX += position.x;
                    PawnDirY += position.y;
                    PawnDirZ += position.z;
                }
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            direction = glm::vec3(PawnDirX - PawnX, PawnDirY - PawnY, PawnDirZ - PawnZ);
            direction = normalize(direction);

            position = direction * pawnSpeed;
            newX = PawnX - position.x;
            newY = PawnY - position.y;
            newZ = PawnZ - position.z;

            currMoveTime = glfwGetTime();
            checkIfValid = check(newX, newY, newZ);
            if (checkIfValid == 2)
            {
                if ((currMoveTime - prevMoveTime) > 0.001)
                {
                    prevMoveTime = currMoveTime;

                    PawnX = newX;
                    PawnY = newY;
                    PawnZ = newZ;
                    camPos -= position;

                    PawnDirX -= position.x;
                    PawnDirY -= position.y;
                    PawnDirZ -= position.z;
                }
            }
        }

        //turning on or off transparency
        else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            alpha = 0.7;
        }
        else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
            alpha = 1.0;
        }

        //increasing or descreasing speed of the pawn
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (pawnSpeed < 0.015)
                pawnSpeed += 0.001f;
        }
        else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            if (pawnSpeed >= 0.004)
                pawnSpeed -= 0.001f;
        }

        //checking if there has been a crush with any obstacle
        if (movingObstacles.crushWithPawn()){
            float finishTime = glfwGetTime();
            std::cout<<"There has been a crush. Your final points: "<<PawnZ / (finishTime - startTime) * 20 / 0.14814815 + points<<"\n";
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
    glfwTerminate();
    return 0;
}