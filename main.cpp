#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <string>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

float planetRotationAngles[9] = {0}; // Spin angles
float planetOrbitAngles[9] = {0};    // Orbit angles

void drawStars();
void drawCircle(float cx, float cy, float radius, int num_segments);
void drawFilledCircle(float cx, float cy, float radius, float r, float g, float b, float rotationAngle);
void drawOrbit(float cx, float cy, float radius);
void display();
void reshape(int w, int h);
void init();
void onMouseMove(int x, int y);
void renderText(float x, float y, const char* text);
void update(int value);

struct Planet {
    float x, y;
    float radius;
    float r, g, b;
    const char* name;
    const char* size;
    const char* distance;
    float rotationAngle;
};

const int NUM_STARS = 150;

// Add with other structs
struct Star {
    float x, y;
};
Star stars[NUM_STARS];

float centerX = WINDOW_WIDTH / 2.0f;
float centerY = WINDOW_HEIGHT / 2.0f;

float orbitRadii[9] = {40, 80, 120, 160, 210, 260, 310, 360, 390};
float planetSizes[9] = {6, 10, 12, 9, 22, 20, 18, 17, 10};
const char* planetNames[9] = {
    "Mercury", "Venus", "Earth", "Mars", "Jupiter",
    "Saturn", "Uranus", "Neptune", "Pluto"
};
const char* planetDistances[9] = {
    "57.9M km", "108.2M km", "149.6M km", "227.9M km",
    "778.3M km", "1.4B km", "2.9B km", "4.5B km", "5.9B km"
};
const char* planetSizesText[9] = {
    "4,879 km", "12,104 km", "12,742 km", "6,779 km",
    "139,820 km", "116,460 km", "50,724 km", "49,244 km", "2,376 km"
};

// Add with other planet info (near top)
const char* sunName = "Sun";
const char* sunSize = "1,391,400 km";
const char* sunDistance = "0 km (Center)";
float sunRadius = 30.0f;

float planetColors[9][3] = {
    {0.6f, 0.6f, 0.6f}, {1.0f, 0.5f, 0.0f}, {0.2f, 0.6f, 1.0f},
    {0.9f, 0.3f, 0.2f}, {0.9f, 0.7f, 0.5f}, {0.95f, 0.85f, 0.6f},
    {0.6f, 0.9f, 1.0f}, {0.3f, 0.5f, 1.0f}, {0.8f, 0.7f, 0.6f}
};

Planet planets[10];
Planet* hoveredPlanet = nullptr;
int mouseX = 0, mouseY = 0;

void renderText(float x, float y, const char* text) {
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void onMouseMove(int x, int y) {
    mouseX = x;
    mouseY = WINDOW_HEIGHT - y;
    hoveredPlanet = nullptr;

    // First check if hovering over Sun
    float dx = mouseX - centerX;
    float dy = mouseY - centerY;
    float distanceToSun = sqrt(dx*dx + dy*dy);

    if(distanceToSun <= sunRadius) {
        // Create temporary Sun "planet" for hover text
        static Planet sunInfo = {
            centerX, centerY,
            sunRadius,
            1.0f, 0.8f, 0.0f, // Sun color
            sunName,
            sunSize,
            sunDistance,
            0.0f // No rotation needed
        };
        hoveredPlanet = &sunInfo;
        glutPostRedisplay();
        return;
    }

    // Then check planets as before
    for(int i = 0; i < 9; i++) {
        dx = mouseX - planets[i].x;
        dy = mouseY - planets[i].y;
        float distance = sqrt(dx*dx + dy*dy);
        if(distance <= planets[i].radius) {
            hoveredPlanet = &planets[i];
            break;
        }
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(0.0, 0.0, 0.1, 0.0);
    srand((unsigned)time(NULL));

    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (float)(rand() % WINDOW_WIDTH);
        stars[i].y = (float)(rand() % WINDOW_HEIGHT);
    }
}

void drawStars() {
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(2.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; i++) {
        glVertex2f(stars[i].x, stars[i].y);
    }
    glEnd();
}

void drawCircle(float cx, float cy, float radius, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * 3.1415926f * i / num_segments;
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}

void drawFilledCircle(float cx, float cy, float radius, float r, float g, float b, float rotationAngle) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotationAngle, 0, 0, 1);

    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 50; i++) {
        float angle = 2.0f * 3.1415926f * i / 50;
        float x = radius * cosf(angle);
        float y = radius * sinf(angle);
        glVertex2f(x, y);
    }
    glEnd();

    glColor3f(r*0.5, g*0.5, b*0.5);
    glBegin(GL_QUADS);
    glVertex2f(-radius, 0.01 * -radius);
    glVertex2f(radius, 0.01 * -radius);
    glVertex2f(radius, 0.01 * radius);
    glVertex2f(-radius, 0.01 * radius);
    glEnd();

    glPopMatrix();
}

void drawOrbit(float cx, float cy, float radius) {
    glColor3f(0.4f, 0.4f, 0.4f);
    drawCircle(cx, cy, radius, 100);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawStars();
    drawFilledCircle(centerX, centerY, 30, 1.0f, 0.8f, 0.0f, 0); // Sun

    for (int i = 0; i < 9; i++) {
        drawOrbit(centerX, centerY, orbitRadii[i]);

        float angleRad = planetOrbitAngles[i] * (3.1415926f / 180.0f);
        float x = centerX + orbitRadii[i] * cos(angleRad);
        float y = centerY + orbitRadii[i] * sin(angleRad);

        drawFilledCircle(x, y, planetSizes[i], planetColors[i][0], planetColors[i][1], planetColors[i][2], planetRotationAngles[i]);

        planets[i] = {
            x, y,
            planetSizes[i],
            planetColors[i][0], planetColors[i][1], planetColors[i][2],
            planetNames[i],
            planetSizesText[i],
            planetDistances[i],
            planetRotationAngles[i]
        };
    }

    if (hoveredPlanet) {
        std::string info = std::string(hoveredPlanet->name) +
            " | Size: " + hoveredPlanet->size +
            " | Distance: " + hoveredPlanet->distance;
        renderText(mouseX + 10, mouseY + 10, info.c_str());
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void update(int value) {
    for (int i = 0; i < 9; i++) {
        planetRotationAngles[i] += (i + 1) * 0.5f;
        if (planetRotationAngles[i] > 360.0f) planetRotationAngles[i] -= 360.0f;

        planetOrbitAngles[i] += (i + 1) * 0.05f;
        if (planetOrbitAngles[i] > 360.0f) planetOrbitAngles[i] -= 360.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(200, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Solar System | Yusuf Sameh 221001112, Ahmed Alaa Mahmoud 221000940, Shadi Muhammad 221010958");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(onMouseMove);
    init();
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
