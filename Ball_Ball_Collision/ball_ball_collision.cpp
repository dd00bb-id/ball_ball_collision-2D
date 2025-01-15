#include <windows.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>

#define PI 3.1415926
#define epsilon 1.0

// Size of Clipping Window
float radius = 5.0;
int width = 500;
int height = 500;
float point_size = 3.0;
int n = 0;

// Parameters of a Sphere
int num = 36;
int c_num = 8; // Number of balls
float delta;
GLenum draw_type;
int collide = 0;

float left = -10.0;
float right = 10.0;
float bottom = -10.0;
float top = 10.0;

struct Point {
    float x;
    float y;
};

struct Vector {
    float x;
    float y;
};

struct Color {
    float r;
    float g;
    float b;
};

struct Circle {
    Point center;
    float radius;
    float mass;
    Color color;
    Vector velocity;
};

Circle* circle;
Point* vertex;

float delta_x, delta_y;
float fix_radius;

Point Window_Center;
float Window_radius;

int collision_count = 0;  // To count the number of collisions

// Initialize OpenGL
void init(void) {
    glClearColor(1.0, 1.0, 1.0, 0.0); // Set display-window color to white
    glMatrixMode(GL_PROJECTION); // Set projection parameters
    gluOrtho2D(left, right, bottom, top);
    draw_type = GL_LINES;

    vertex = new Point[num];
    delta = 2.0 * PI / num;
    for (int i = 0; i < num; i++) {
        vertex[i].x = cos(delta * i);
        vertex[i].y = sin(delta * i);
    }

    Window_radius = width / 3.0;
    Window_Center.x = width / 2.0;
    Window_Center.y = height / 2.0;

    delta = 2.0 * PI / c_num;
    circle = new Circle[c_num];

    for (int i = 0; i < c_num; i++) {
        // Set random properties for each circle
        circle[i].radius = 0.5 + rand() % 2; // Random radius (1.0~3.0)
        circle[i].center.x = (rand() % 20) - 10.0; // Random x position (-10 ~ 10)
        circle[i].center.y = (rand() % 20) - 10.0; // Random y position (-10 ~ 10)
        circle[i].velocity.x = (rand() % 10 - 5) * 0.001; // Random velocity (-0.05 ~ 0.05)
        circle[i].velocity.y = (rand() % 10 - 5) * 0.001; // Random velocity (-0.05 ~ 0.05)
        circle[i].mass = 1.0 + rand() % 3; // Random mass (1.0~3.0)

        // Random color
        circle[i].color.r = (rand() % 100) / 100.0f;
        circle[i].color.g = (rand() % 100) / 100.0f;
        circle[i].color.b = (rand() % 100) / 100.0f;
    }
}

// Function to draw a circle
void Draw_Circle(int index) {
    float x, y, radius;
    x = circle[index].center.x;
    y = circle[index].center.y;
    radius = circle[index].radius;
    glColor3f(circle[index].color.r, circle[index].color.g, circle[index].color.b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < num; i++) {
        glVertex3f(x + radius * vertex[i].x, y + radius * vertex[i].y, 0.0);
    }
    glEnd();
}

// Window resize callback
void MyReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(left, right, bottom, top);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Ball-Wall collision detection
void Check_Collision_Ball_Wall(int index) {
    if (circle[index].center.x + circle[index].radius > right && circle[index].velocity.x > 0.0)
        circle[index].velocity.x *= (-1.0);

    if (circle[index].center.x - circle[index].radius < left && circle[index].velocity.x < 0.0)
        circle[index].velocity.x *= (-1.0);

    if (circle[index].center.y + circle[index].radius > top && circle[index].velocity.y > 0.0)
        circle[index].velocity.y *= (-1.0);

    if (circle[index].center.y - circle[index].radius < bottom && circle[index].velocity.y < 0.0)
        circle[index].velocity.y *= (-1.0);
}

// Ball-Ball collision detection
void Check_Collision_Ball_Ball(int i, int j) {
    float distance;
    Vector VA, VB, R_AB, normal;
    float inner_value;

    normal.x = circle[j].center.x - circle[i].center.x;
    normal.y = circle[j].center.y - circle[i].center.y;

    distance = sqrt(normal.x * normal.x + normal.y * normal.y);

    if (distance < circle[j].radius + circle[i].radius) {
        normal.x /= distance;
        normal.y /= distance;

        VA.x = circle[i].velocity.x;
        VA.y = circle[i].velocity.y;

        VB.x = circle[j].velocity.x;
        VB.y = circle[j].velocity.y;

        R_AB.x = VA.x - VB.x;
        R_AB.y = VA.y - VB.y;

        inner_value = R_AB.x * normal.x + R_AB.y * normal.y;
        if (inner_value > 0.0) {
            circle[i].velocity.x = VA.x - (1 + epsilon) * circle[j].mass * inner_value * normal.x / (circle[i].mass + circle[j].mass);
            circle[i].velocity.y = VA.y - (1 + epsilon) * circle[j].mass * inner_value * normal.y / (circle[i].mass + circle[j].mass);

            circle[j].velocity.x = VB.x + (1 + epsilon) * circle[i].mass * inner_value * normal.x / (circle[i].mass + circle[j].mass);
            circle[j].velocity.y = VB.y + (1 + epsilon) * circle[i].mass * inner_value * normal.y / (circle[i].mass + circle[j].mass);

            collide = 1;
            collision_count++; // Increment collision count
        }
    }
}

// Update ball positions
void Update_Position(void) {
    for (int i = 0; i < c_num; i++) {
        circle[i].center.x += circle[i].velocity.x;
        circle[i].center.y += circle[i].velocity.y;
    }
}

// Display callback function
void RenderScene(void) {
    int i;

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);

    // Change background color when a collision occurs
    if (collide) {
        glClearColor(1.0, 0.0, 0.0, 0.0);  // Set background color to red
    }

    // Handle wall collisions
    for (i = 0; i < c_num; i++)
        Check_Collision_Ball_Wall(i);

    Update_Position();

    // Check ball-to-ball collisions
    for (i = 0; i < c_num; i++) {
        for (int j = i + 1; j < c_num; j++) {
            if (i != j)
                Check_Collision_Ball_Ball(i, j);
        }
    }

    Update_Position();

    // Draw the balls
    for (i = 0; i < c_num; i++)
        Draw_Circle(i);

    // Display collision count
    glColor3f(0.0, 0.0, 0.0);
    char buffer[50];
    sprintf(buffer, "Collisions: %d", collision_count);
    glRasterPos2f(-9.0, 9.0);  // Display at top-left corner
    for (int i = 0; buffer[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
    }

    glFlush();
    glutSwapBuffers();
}

// Key press function (to modify ball speed)
void SpecialKey(int key, int x, int y) {
    int i;

    switch (key) {
    case GLUT_KEY_LEFT:
        for (i = 0; i < c_num; i++) {
            circle[i].velocity.x *= 0.1;
            circle[i].velocity.y *= 0.1;
        }
        break;
    case GLUT_KEY_RIGHT:
        for (i = 0; i < c_num; i++) {
            circle[i].velocity.x *= 1.5;
            circle[i].velocity.y *= 1.5;
        }
        break;
    default:
        break;
    }
}

// Idle function to keep rendering the scene
void IdleFunction(void) {
    RenderScene();
    glutPostRedisplay();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Ball_Ball_Collision");
    init();
    glutDisplayFunc(RenderScene);
    glutReshapeFunc(MyReshape);
    glutSpecialFunc(SpecialKey);
    glutIdleFunc(IdleFunction);
    glutMainLoop();
    return 0;
}
