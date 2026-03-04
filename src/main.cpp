#include <SDL.h>
#include <GL/gl.h>
#include <cmath>
#include <string>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// ─── Math helpers ──────────────────────────────────────────────────────────────

struct Vec3 { float x, y, z; };

static void matMul(const float a[16], const float b[16], float out[16]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            out[r*4+c] = 0;
            for (int k = 0; k < 4; k++)
                out[r*4+c] += a[r*4+k] * b[k*4+c];
        }
}

static void identity(float m[16]) {
    for (int i=0;i<16;i++) m[i]=0;
    m[0]=m[5]=m[10]=m[15]=1;
}

static void perspective(float m[16], float fovY, float aspect, float zn, float zf) {
    float f = 1.0f / tanf(fovY * 0.5f);
    identity(m);
    m[0]  =  f / aspect;
    m[5]  =  f;
    m[10] =  (zf + zn) / (zn - zf);
    m[11] = -1.0f;
    m[14] =  (2.0f * zf * zn) / (zn - zf);
    m[15] =  0.0f;
}

static void rotateX(float m[16], float a) {
    identity(m);
    m[5]  =  cosf(a); m[6]  = sinf(a);
    m[9]  = -sinf(a); m[10] = cosf(a);
}
static void rotateY(float m[16], float a) {
    identity(m);
    m[0]  =  cosf(a); m[2]  = -sinf(a);
    m[8]  =  sinf(a); m[10] =  cosf(a);
}
static void rotateZ(float m[16], float a) {
    identity(m);
    m[0]  =  cosf(a); m[1]  = sinf(a);
    m[4]  = -sinf(a); m[5]  = cosf(a);
}
static void translate(float m[16], float tx, float ty, float tz) {
    identity(m);
    m[12] = tx; m[13] = ty; m[14] = tz;
}

// ─── Cube geometry ─────────────────────────────────────────────────────────────
//  8 vertices, 6 faces (each 2 triangles)

static const float cubeVerts[8][3] = {
    {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
    {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1},
};

// face normal directions (for diffuse shading)
static const float faceNormals[6][3] = {
    { 0, 0,-1}, { 0, 0, 1},
    {-1, 0, 0}, { 1, 0, 0},
    { 0,-1, 0}, { 0, 1, 0},
};

// 4 vertex indices per face
static const int faceIdx[6][4] = {
    {0,1,2,3}, {5,4,7,6},
    {4,0,3,7}, {1,5,6,2},
    {4,5,1,0}, {3,2,6,7},
};

// Face base colors (RGB)
static const float faceColors[6][3] = {
    {0.96f,0.26f,0.21f},  // red   -Z
    {0.13f,0.59f,0.95f},  // blue  +Z
    {0.30f,0.69f,0.31f},  // green -X
    {1.00f,0.76f,0.03f},  // yellow+X
    {1.00f,0.34f,0.13f},  // orange-Y
    {1.00f,1.00f,1.00f},  // white +Y
};

// ─── Draw cube with fixed-function OpenGL ──────────────────────────────────────

static void drawCube(float modelview[16], float proj[16], bool wireframe, float wireAlpha) {
    // Combine MVP
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelview);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Simple diffuse: light direction (world)
    float light[3] = {0.6f, 0.8f, 0.4f};
    float llen = sqrtf(light[0]*light[0]+light[1]*light[1]+light[2]*light[2]);
    light[0]/=llen; light[1]/=llen; light[2]/=llen;

    // ── Solid faces ────────────────────────────────────────────────────────
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonOffset(1.0f, 1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; f++) {
        const float* n = faceNormals[f];
        float diff = n[0]*light[0] + n[1]*light[1] + n[2]*light[2];
        if (diff < 0) diff = 0;
        float amb  = 0.25f;
        float lum  = amb + (1.0f - amb) * diff;
        glColor3f(faceColors[f][0]*lum, faceColors[f][1]*lum, faceColors[f][2]*lum);
        for (int v = 0; v < 4; v++) {
            const float* p = cubeVerts[faceIdx[f][v]];
            glVertex3f(p[0], p[1], p[2]);
        }
    }
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // ── Wireframe edges ────────────────────────────────────────────────────
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glColor4f(0.05f, 0.05f, 0.05f, wireAlpha);
        glBegin(GL_QUADS);
        for (int f = 0; f < 6; f++)
            for (int v = 0; v < 4; v++) {
                const float* p = cubeVerts[faceIdx[f][v]];
                glVertex3f(p[0], p[1], p[2]);
            }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// ─── Entry point ───────────────────────────────────────────────────────────────

int main(int /*argc*/, char** /*argv*/) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        "Cube Demo — ImGui + SDL2 + OpenGL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Dark style with custom colours
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 10.0f;
    style.FrameRounding     = 6.0f;
    style.GrabRounding      = 6.0f;
    style.WindowBorderSize  = 0.0f;
    style.FramePadding      = {10, 6};
    style.ItemSpacing       = {8, 8};

    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]       = {0.09f,0.09f,0.12f,0.95f};
    c[ImGuiCol_Header]         = {0.20f,0.40f,0.80f,0.45f};
    c[ImGuiCol_HeaderHovered]  = {0.20f,0.40f,0.80f,0.80f};
    c[ImGuiCol_SliderGrab]     = {0.36f,0.65f,1.00f,1.00f};
    c[ImGuiCol_SliderGrabActive]={0.50f,0.80f,1.00f,1.00f};
    c[ImGuiCol_FrameBg]        = {0.15f,0.15f,0.22f,1.00f};
    c[ImGuiCol_Button]         = {0.20f,0.40f,0.80f,0.60f};
    c[ImGuiCol_ButtonHovered]  = {0.30f,0.55f,1.00f,0.80f};
    c[ImGuiCol_TitleBgActive]  = {0.15f,0.25f,0.55f,1.00f};

    ImGui_ImplSDL2_InitForOpenGL(window, glctx);
    ImGui_ImplOpenGL3_Init("#version 120");

    // ── State ──────────────────────────────────────────────────────────────
    float rotX = 0.3f, rotY = 0.5f, rotZ = 0.0f;
    float autoSpeedX = 0.5f, autoSpeedY = 0.8f, autoSpeedZ = 0.0f;
    bool  autoRotate  = true;
    bool  wireframe   = true;
    float bgColor[4]  = {0.08f, 0.08f, 0.11f, 1.00f};
    float fovDeg      = 60.0f;
    float camDist     = 5.0f;
    float wireAlpha   = 0.85f;
    bool  running     = true;
    Uint64 prevTime   = SDL_GetPerformanceCounter();

    while (running) {
        // ── Delta time ─────────────────────────────────────────────────────
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = float(now - prevTime) / float(SDL_GetPerformanceFrequency());
        prevTime = now;

        // ── Events ─────────────────────────────────────────────────────────
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // ── Auto-rotation ──────────────────────────────────────────────────
        if (autoRotate) {
            rotX += autoSpeedX * dt;
            rotY += autoSpeedY * dt;
            rotZ += autoSpeedZ * dt;
        }

        // ── Build matrices ─────────────────────────────────────────────────
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        float aspect = (h > 0) ? float(w) / float(h) : 1.0f;

        float proj[16], T[16], Rx[16], Ry[16], Rz[16], tmp[16], model[16];
        perspective(proj, fovDeg * 3.14159f / 180.0f, aspect, 0.1f, 100.0f);
        translate(T, 0, 0, -camDist);
        rotateX(Rx, rotX);
        rotateY(Ry, rotY);
        rotateZ(Rz, rotZ);

        // model = Rz * Ry * Rx
        matMul(Ry, Rx, tmp);
        matMul(Rz, tmp, model);
        // modelview = T * model
        float mv[16];
        matMul(T, model, mv);

        // ── Clear ──────────────────────────────────────────────────────────
        glViewport(0, 0, w, h);
        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ── Draw cube ──────────────────────────────────────────────────────
        drawCube(mv, proj, wireframe, wireAlpha);

        // ── ImGui ──────────────────────────────────────────────────────────
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({20, 20}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({330, 0}, ImGuiCond_Once);
        ImGui::Begin("  Cube Controls", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::SeparatorText("Rotation");
        ImGui::Checkbox("Auto-rotate", &autoRotate);
        if (autoRotate) {
            ImGui::SliderFloat("Speed X", &autoSpeedX, -3.0f, 3.0f);
            ImGui::SliderFloat("Speed Y", &autoSpeedY, -3.0f, 3.0f);
            ImGui::SliderFloat("Speed Z", &autoSpeedZ, -3.0f, 3.0f);
        } else {
            ImGui::SliderFloat("Angle X", &rotX, -3.14159f, 3.14159f);
            ImGui::SliderFloat("Angle Y", &rotY, -3.14159f, 3.14159f);
            ImGui::SliderFloat("Angle Z", &rotZ, -3.14159f, 3.14159f);
        }
        if (ImGui::Button("Reset rotation")) { rotX = rotY = rotZ = 0; }

        ImGui::SeparatorText("Camera");
        ImGui::SliderFloat("FOV",      &fovDeg,  20.0f, 120.0f);
        ImGui::SliderFloat("Distance", &camDist,  2.0f,  15.0f);

        ImGui::SeparatorText("Rendering");
        ImGui::Checkbox("Wireframe", &wireframe);
        if (wireframe)
            ImGui::SliderFloat("Wire alpha", &wireAlpha, 0.0f, 1.0f);
        ImGui::ColorEdit3("Background", bgColor);

        ImGui::SeparatorText("Info");
        ImGui::Text("FPS : %.1f", io.Framerate);
        ImGui::Text("dt  : %.2f ms", dt * 1000.0f);
        ImGui::Text("ESC to quit");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
