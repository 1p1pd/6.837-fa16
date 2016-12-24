#include "gl.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <lodepng.h>
#include <map>
#include <cstdint>

#include "objparser.h"

// some utility code is tucked away in main.h
// for example, drawing the coordinate axes
// or helpers for setting uniforms.
#include "main.h"

// 4096x4096 is a pretty large texture. Extensions to shadow algorithm
// (extra credit) help lowering this memory footprint.
const int SHADOW_WIDTH = 4096;
const int SHADOW_HEIGHT = 4096;

// FUNCTION DECLARATIONS - you will implement these
void loadTextures();
void freeTextures();

void loadFramebuffer();
void freeFramebuffer();

void draw();

Matrix4f getLightView();
Matrix4f getLightProjection();

// Globals here.
objparser scene;
Vector3f  light_dir;
glfwtimer timer;
std::map<std::string, GLuint> gltextures;
GLuint fb;
GLuint fb_depthtex;
GLuint fb_colortex;

// FUNCTION IMPLEMENTATIONS

// animate light source direction
// this one is implemented for you
void updateLightDirection() {
    // feel free to edit this
    float elapsed_s = timer.elapsed();
    //elapsed_s = 88.88f;
    float timescale = 0.1f;
    light_dir = Vector3f(2.0f * sinf((float)elapsed_s * 1.5f * timescale),
        5.0f, 2.0f * cosf(2 + 1.9f * (float)elapsed_s * timescale));
    light_dir.normalize();
}


void drawScene(GLint program, Matrix4f V, Matrix4f P) {
    VertexRecorder rec;
    Matrix4f M = Matrix4f::identity();
    updateTransformUniforms(program, M, V, P);
    for (unsigned i = 0; i < scene.batches.size(); ++i) {
        draw_batch b = scene.batches[i];
        for (int j = b.start_index; j < b.start_index + b.nindices; ++j) {
            int index = scene.indices[j];
            rec.record(scene.positions[index],
                       scene.normals[index],
                       Vector3f(scene.texcoords[index], 0));
        }
        updateMaterialUniforms(program, b.mat.diffuse, b.mat.ambient, b.mat.specular, b.mat.shininess);
        GLuint gltexture = gltextures[b.mat.diffuse_texture];
        glBindTexture(GL_TEXTURE_2D, gltexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fb_depthtex);
        glActiveTexture(GL_TEXTURE0);
        int loc = glGetUniformLocation(program, "shadowTex");
        glUniform1i(loc, 1);
        rec.draw();
    }
}

void loadTextures() {
    for (auto it = scene.textures.begin(); it != scene.textures.end() ; ++it) {
        std::string name = it->first;
        rgbimage& im = it->second;
        GLuint gltexture;
        glGenTextures(1, &gltexture);
        glBindTexture(GL_TEXTURE_2D, gltexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, im.w, im.h, 0, GL_RGB, GL_UNSIGNED_BYTE, im.data.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gltextures.insert(std::make_pair(name, gltexture));
    }
}

void freeTextures() {
    for (auto it = scene.textures.begin(); it !=scene.textures.end() ; ++it) {
        std::string name = it->first;
        GLuint gltexture = gltextures[name];
        glDeleteTextures(1, &gltexture);
    }
}

void loadFramebuffer() {
    glGenTextures(1, &fb_colortex);
    glBindTexture(GL_TEXTURE_2D, fb_colortex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4096, 4096, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenTextures(1, &fb_depthtex);
    glBindTexture(GL_TEXTURE_2D, fb_depthtex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_colortex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_depthtex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: incomplete framebuffer\n");
        exit(-1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void freeFramebuffer() {
    glDeleteTextures(1, &fb_colortex);
    glDeleteTextures(1, &fb_depthtex);
    glDeleteFramebuffers(1, &fb);
}

Matrix4f getLightView() {
    Vector3f eye = Vector3f(0, 1, 0) + 50 * light_dir;
    Vector3f center = Vector3f(0, 1, 0);
    Vector3f up = Vector3f::cross(light_dir, Vector3f(0, 0, -1)).normalized();
    return Matrix4f::lookAt(eye, center, up);
}

Matrix4f getLightProjection() {
    return Matrix4f::orthographicProjection(50, 50, 20, 80);
}

void draw() {
    // 2. DEPTH PASS
    // - bind framebuffer
    // - configure viewport
    // - compute camera matrices (light source as camera)
    // - call drawScene
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glUseProgram(program_color);
    drawScene(program_color, getLightView(), getLightProjection());

    // 1. LIGHT PASS
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int winw, winh;
    glfwGetFramebufferSize(window, &winw, &winh);
    glViewport(0, 0, winw, winh);
    glUseProgram(program_light);
    updateLightUniforms(program_light, light_dir, Vector3f(1.2f, 1.2f, 1.2f));
    // TODO IMPLEMENT drawScene
    int loc = glGetUniformLocation(program_light, "light_VP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, getLightProjection() * getLightView());
    drawScene(program_light, camera.GetViewMatrix(), camera.GetPerspective());

    // 3. DRAW DEPTH TEXTURE AS QUAD
    // drawTexturedQuad() helper in main.h is useful here.
    glViewport(0, 0, 256, 256);
    drawTexturedQuad(fb_depthtex);
    glViewport(256, 0, 256, 256);
    drawTexturedQuad(fb_colortex);
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char* argv[])
{
    std::string basepath = "./";
    if (argc > 2) {
        printf("Usage: %s [basepath]\n", argv[0]);
    }
    else if (argc == 2) {
        basepath = argv[1];
    }
    printf("Loading scene and shaders relative to path %s\n", basepath.c_str());

    // load scene data
    // parsing code is in objparser.cpp
    // take a look at the public interface in objparser.h
    if (!scene.parse(basepath + "data/sponza_low/sponza_norm.obj")) {
        return -1;
    }

    window = createOpenGLWindow(1024, 1024, "Assignment 5");

    // setup the event handlers
    // key handlers are defined in main.h
    // take a look at main.h to know what's in there.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, motionCallback);

    glClearColor(0.8f, 0.8f, 1.0f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    // TODO add loadXYZ() function calls here
    loadTextures();
    loadFramebuffer();

    camera.SetDimensions(600, 600);
    camera.SetPerspective(50);
    camera.SetDistance(10);
    camera.SetCenter(Vector3f(0, 1, 0));
    camera.SetRotation(Matrix4f::rotateY(1.6f) * Matrix4f::rotateZ(0.4f));

    // set timer for animations
    timer.set();
    while (!glfwWindowShouldClose(window)) {
        setViewportWindow(window);

        // we reload the shader files each frame.
        // this shaders can be edited while the program is running
        // loadPrograms/freePrograms is implemented in main.h
        bool valid_shaders = loadPrograms(basepath);
        if (valid_shaders) {

            // draw coordinate axes
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (gMousePressed) {
                drawAxis();
            }

            // update animation
            updateLightDirection();

            // draw everything
            draw();
        }
        // make sure to release the shader programs.
        freePrograms();

        // Make back buffer visible
        glfwSwapBuffers(window);

        // Check if any input happened during the last frame
        glfwPollEvents();
    } // END OF MAIN LOOP

    // All OpenGL resource that are created with
    // glGen* or glCreate* must be freed.
    // TODO: add freeXYZ() function calls here
    freeFramebuffer();
    freeTextures();

    glfwDestroyWindow(window);


    return 0;	// This line is never reached.
}
