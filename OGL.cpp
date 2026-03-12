// standred header files
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

// OpenGL related Header files
#include <GL/glew.h> //this header file must be included before gl/GL.h
#include <gl/GL.h>   //C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\gl path to GL.h file

// Custom files
#include "OGL.h"
#include "Sphere.h"
#include "vmath.h"
#include"stack.h"
using namespace vmath;

// OpenGL Related Libraries
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib") // C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64 OpenGL32.lib

#pragma comment(lib, "sphere.lib")

// window sizes
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variables declarations
BOOL gbFullScreen = FALSE;
HWND ghwnd = NULL;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

// active window variables
BOOL gbActiveWindow = FALSE;

// Exit keypress related
BOOL gbEscapeKeyIsPressed = FALSE;

// variables related File I/O
char gszLogFileName[] = "Log.txt";
FILE *gpFile = NULL;

// opengl related global variables
HDC ghdc = NULL;
HGLRC ghrc = NULL; //(handle to graphics lib rendering context)

// shader related global variables
GLuint shaderProgramObject = 0;

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_TEXCOORD = 1,
};

GLuint gVao_sphere = 0;
GLuint gVbo_sphere_position = 0;
GLuint gVbo_sphere_texcoord = 0;
GLuint gVbo_sphere_element = 0;

GLuint gVao_Cube = 0;
GLuint gVbo_Cube_position = 0;
GLuint gVbo_Cube_texcoord = 0;

GLuint modelMatrixUniform = 0;  // mvp - model view  projection matrix
GLuint viewMatrixUniform = 0;
GLuint ProjectionMatrixUniform = 0;

GLuint textureSamplerUniform = 0;

mat4 PerspectiveProjectionMatrix; //mat = matrix 4 = 4 X 4 matrix , vmath::mat4

//variable related to sphere

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

unsigned int gNumVertices = 0;
unsigned int gNumElements  = 0;

//Texture
//variables related Texture
GLuint Texture_SolarSystem;
GLuint Texture_Sun;
GLuint Texture_Earth;
GLuint Texture_Moon;

//solar system related variables
float fYear = 0.0f;
float fDate = 0.0f;
float fMoon = 0.0f;

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Local Function Declarations
    int initialise(void);
    void display(void);
    void update(void);
    void uninitialise(void);

    // Variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR6");
    BOOL bDone = FALSE;

    // variables related window centering
    int iX_Position;
    int iY_Position;

    // code

    // Create LogFile
    gpFile = fopen(gszLogFileName, "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL,
                   TEXT("LogFile Creation Failed"),
                   TEXT("File I/O Error"),
                   MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program Started Successfully...\n");
    }

    // window class initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // Registraton of Window Class
    if (!RegisterClassEx(&wndclass))
    {
        MessageBox(NULL,
                   TEXT("Window Class Registeration Failed!!!"),
                   TEXT("RegisterClassEx"),
                   MB_ICONERROR);
        return (0);
    }

    // window position x-coordinate and y-coordinate
    iX_Position = (GetSystemMetrics(SM_CXFULLSCREEN)) / 2 - (WIN_WIDTH / 2);
    iY_Position = (GetSystemMetrics(SM_CYFULLSCREEN)) / 2 - (WIN_HEIGHT / 2);

    // create window
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        szAppName,
        TEXT("Shubhangi R Bhadrashette"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        iX_Position,
        iY_Position,
        WIN_WIDTH,
        WIN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ghwnd = hwnd;

    // Show window
    ShowWindow(hwnd, iCmdShow);

    // Paint the background of window
    UpdateWindow(hwnd);

    // initialise
    int result = initialise();
    if (result != 0)
    {
        fprintf(gpFile, "initialise failed....!!!\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "initialise Fun Completed Successfully...\n");
    }

    // set this as foreground and active window
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // game loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActiveWindow == TRUE)
            {
                if (gbEscapeKeyIsPressed == TRUE)
                {
                    bDone = TRUE;
                }
                // render
                display();

                // update
                update();
            }
        }
    }

    uninitialise();
    return ((int)(msg.wParam));
}

// Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // local function declarations
    void toggleFullScreen(void);
    void resize(int, int);
    void uninitialise(void);

    // code
    switch (iMsg)
    {
    case WM_CREATE:
        ZeroMemory((void *)&wpPrev, sizeof(WINDOWPLACEMENT));
        wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

    case WM_SETFOCUS:
        gbActiveWindow = TRUE;
        break;

    case WM_KILLFOCUS:
        gbActiveWindow = FALSE;
        break;

    case WM_ERASEBKGND:
        return (0);

    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            gbEscapeKeyIsPressed = TRUE;
            break;

        default:
            break;
        }
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case 'F':
        case 'f':
            if (gbFullScreen == FALSE)
            {
                toggleFullScreen();
                gbFullScreen = TRUE;
            }
            else
            {
                toggleFullScreen();
                gbFullScreen = FALSE;
            }
            break;

        default:
            break;
        }
        break;

    case WM_CLOSE:
        uninitialise();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    // variable declarations
    MONITORINFO mi;

    // code
    if (gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            ZeroMemory((void *)&mi, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

int initialise(void)
{
    // function declarations
    void printGLInfo(void);
    void resize(int width, int heigth);
    void uninitialise(void);
    BOOL LoadGLTexture(GLuint * , TCHAR []);

    // variables declarations
    PIXELFORMATDESCRIPTOR pfd; // every os has structure for giving info about pixel to os
    int iPixelFormatIndex = 0;
    GLenum glewResult;

    // code

    // pixelformatdescriptor initialization
    ZeroMemory((void *)&pfd, sizeof(PIXELFORMATDESCRIPTOR)); // own function of window to empty struct (set bytes to zero)

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // size of struture (nSize = n id prefix of short int)
    pfd.nVersion = 1;                          // ffp shiktana os ne support kelyalya opengl sobat karavi lagte tyamule pfd la opengl ffp cha recommended version dyav lagal that is 1
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    // mala window varti draw karaich aahe |
    // render mode api use karaich aahe  not retained mode (gdi graphic nahi karaich ) he sagat |
    // single buffering ekach buffer madhe tom annd bachhan dakhvaich
    // double buffering = front buffer samor aahe te and je swap karaich te  cccccxcxxcxback buffer
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    // GetDc
    ghdc = GetDC(ghwnd);
    if (ghdc == NULL)
    {
        fprintf(gpFile, " ERROR :GetDC() function failed\n");
        return (-1);
    }

    // Get Matching pixel format index using hdc and pfd
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        fprintf(gpFile, " ERROR : ChoosePixelFormat(): failed\n");
        return (-2);
    }

    // select the pixel formats of found index
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "ERROR: SetPixelFormat() Function Failed..!!");
        return (-3);
    }

    // Create Rendering Context Using hdc pfd And PixelFormatRendering
    ghrc = wglCreateContext(ghdc); // bridging api now a days it is called as WSI (window system integration)
    if (ghrc == NULL)
    {
        fprintf(gpFile, "ERROR: wglCreateContext() Function Failed !!");
        return (-4);
    }

    // make this rendering context as current context
    if (wglMakeCurrent(ghdc, ghrc) == FALSE) // aata current context ha rendering context set zala
    {
        fprintf(gpFile, "ERROR: wglMakeCurrent() Function Failed !!");
        return (-5);
    }

    // initialise glew
    glewResult = glewInit();
    if (glewResult != GLEW_OK)
    {
        fprintf(gpFile, "ERROR: glewInit() Function Failed !!");
        return (-6);
    }

    // print gl info
    printGLInfo();

    // From here onword OpenGL Code Start

    //------------------------------------------------------------------------------------------------------------------------------------
    // Vertex Shader = compulsary shaders

    // 1) write the shader source code ->
    const GLchar *vertexShaderSourceCode =
        "#version 460 core\n" // 1st line shoud be openGL Version: 4.6 * 100 = 460
        "in vec4 aPosition;\n"  //4 member aasnara (4ch array aasnara) konitari shader chya aata yet aahe tyatch nav aapn aposition dil aahe //glsl chya compiler la kalel he c and c++ compiler la nahi kalnar
        "uniform mat4 uModelMatrix;\n"  //jo uniform aahe aasa mat4(4X4 matrix) type cha jyala aapl uMVPMatrix nav dil aahe to ye aahe ,vmath::mat4 yacha and GLSL cha mat4 vegle vegle aahet
        "uniform mat4 uViewMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "in vec2 aTexcoord;\n"
        "out vec2 out_Texcoord;\n"
        "void main(void)\n"
        "{\n"
            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"
             "out_Texcoord = aTexcoord;\n"
        "}\n";

    // 2) create the shader object ->
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER); // he function shader object tayar karat

    // 3) give the shader source code to the shader object ->
    glShaderSource(vertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    // 1st which shader object 2nd how many shaders are giving 3rd array of shaders 4th array of lengths of every shader

    // 4) compile the shader->
    glCompileShader(vertexShaderObject);

    // 5) do shader compilation error checking
    GLint status = 0;         // success zal ka nahi compilation he store karnyasathi
    GLint infoLogLength = 0;  // log info denari string chi length
    GLchar *szInfoLog = NULL; // error string store karnyasathi

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) // error aasel tr aat ya
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0) // there are some error which does not have length which does not have string but still thay are logical error it must be solved by mannually
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, NULL, szInfoLog); // 3rd para actual length of error string det
                fprintf(gpFile, "Vertex Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialise();
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    // Fragment shader = compulsary shaders

    // 1) write the shader source code ->
    const GLchar *fragmentShaderSourceCode =
        "#version 460 core\n"
        "in vec2 out_Texcoord;\n"
        "uniform sampler2D uTextureSampler;\n"
        "out vec4 FragColor;\n" //out = shader madhun jatoy and perfragment operation la jayo -> framebuffer
        "void main(void)\n"
        "{ \n" 
            "FragColor = texture(uTextureSampler, out_Texcoord);\n"  //per vertex la white color deto,   pp madhe default white color nasto ffp sarkh
        "} \n";

    // 2) create the shader object ->
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    // 3) give the shader source code to the shader object ->
    glShaderSource(fragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

    // 4) compile the shader->
    glCompileShader(fragmentShaderObject);

    // 5) do shader compilation error checking
    status = 0;        // success zal ka nahi compilation he store karnyasathi
    infoLogLength = 0; // log info denari string chi length
    szInfoLog = NULL;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialise();
    }

    // 6) create shader program object
    shaderProgramObject = glCreateProgram();

    // 7) attach shader object to shader program object
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    // bind shader attribute at a certain index in shader to same index in host program
    glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_VERTEX, "aPosition");  
    glBindAttribLocation(shaderProgramObject, VDG_ATTRIBUTE_TEXCOORD, "aTexcoord");  

    // 8) tell to link shader objects to shader program objects
    glLinkProgram(shaderProgramObject);

    // 9) check for link error logs
    status = 0;        // success zal ka nahi compilation he store karnyasathi
    infoLogLength = 0; // log info denari string chi length
    szInfoLog = NULL;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(infoLogLength * sizeof(GLchar));
            if (szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, NULL, szInfoLog);
                fprintf(gpFile, "Shader Program Link  Log = %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialise();
    }
    //---------------------------------------------------------------------------------------------------------------------------
   
    // get the required uniform location from the shader  
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    ProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    uTextureSampler = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    const GLfloat cube_Position[] =
    {
        5.0f,  4.0f,  -1.0f, // top-right of front
        -5.0f,  4.0f,  -1.0f, // top-left of front
        -5.0f, -4.0f,  -1.0f, // bottom-left of front
        5.0f, -4.0f,  -1.0f, // bottom-right of front
    };

    const GLfloat Cube_Texcoord[] = 
    {
        1.0f, 1.0f, // top-right of front
        0.0f, 1.0f, // top-left of front
        0.0f, 0.0f, // bottom-left of front
        1.0f, 0.0f, // bottom-right of front
    };

    // override / fill texcoords ourselves
    for (int i = 0; i < gNumVertices; ++i)
    {
        float x = sphere_vertices[3 * i + 0];
        float y = sphere_vertices[3 * i + 1];
        float z = sphere_vertices[3 * i + 2];

        // normalize vertex position (x, y, z) as further calculations are done for unit sphere ( radius = 1.0)
        float len = sqrtf(x*x + y*y + z*z);
        if (len > 0.0f) {
            x /= len;
            y /= len;
            z /= len;
        }

        //  texture u coordinates they lie on longitude
        // so first find the angle of the point around Y axis wrt +x axis on longitude
        // (Simple horizontal angle around the spehre)
        float longAngle = atan2f(z, x);
        // this anngle is between -PI to PI, so normlise to get u between 0 to 1
        float u = longAngle / (M_PI * 2) + 0.5f;   // [0,1]
        // same logic for v, ( It lies on lattitude)
        float v = 0.5f - asinf(y) / M_PI;          // [0,1]
        sphere_textures[2 * i + 0] = u;
        sphere_textures[2 * i + 1] = v;
    }

    // vao
    glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);

    // position vbo
    glGenBuffers(1, &gVbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // normal vbo
    glGenBuffers(1, &gVbo_sphere_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_textures), sphere_textures, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // element vbo
    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);

//-----------------------------------------------------------------------------------

    // vao
    glGenVertexArrays(1, &gVao_Cube);
    glBindVertexArray(gVao_Cube);

    // position vbo
    glGenBuffers(1, &gVbo_Cube_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_Cube_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_Position), cube_Position, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // texcoord vbo
    glGenBuffers(1, &gVbo_Cube_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_Cube_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cube_Texcoord), Cube_Texcoord, GL_STATIC_DRAW);

    glVertexAttribPointer(VDG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // unbind vao
    glBindVertexArray(0);
//------------------------------------------------------------------------------------

    // Depth Related Function Call
    glClearDepth(1.0f); // display madhe gelyavr mazya depth buffer la 1 kr
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // LEQUAL = Less Than Or Equal , Aashya Fragment la pass kr jyachi value less than or equal to 1 aasel

    // Tell OpenGL to choose the color to clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Load Texture
    if(LoadGLTexture(&Texture_SolarSystem, MAKEINTRESOURCE(IDBITMAP_SOLARSYSTEAM)) == FALSE)
    {
        fprintf(gpFile, " LoadGLTexture(); Failed To Load solarsysteam Texture");
        return(-7);
    }
    
    if(LoadGLTexture(&Texture_Sun,MAKEINTRESOURCE(IDBITMAP_SUN)) == FALSE)
    {
        fprintf(gpFile, " LoadGLTexture(); Failed To Load sun Texture");
        return(-8);
    } 
    
    if(LoadGLTexture(&Texture_Earth,MAKEINTRESOURCE(IDBITMAP_EARTH)) == FALSE)
    {
        fprintf(gpFile, " LoadGLTexture(); Failed To Load earth Texture");
        return(-9);
    }
    
    if(LoadGLTexture(&Texture_Moon,MAKEINTRESOURCE(IDBITMAP_MOON)) == FALSE)
    {
        fprintf(gpFile, " LoadGLTexture(); Failed To Load moon Texture");
        return(-10);
    }

    //Enable Texturing
    glEnable(GL_TEXTURE_2D);

    PerspectiveProjectionMatrix = mat4::identity(); // analogous to opengl loadidentity() in resize

    // Wormup Resize
    resize(WIN_WIDTH, WIN_HEIGHT);
    return (0);
}

BOOL LoadGLTexture(GLuint *Texture, TCHAR ImageResourceID[])
{
    //Variable declaration
    HBITMAP hBitmap = NULL;
    BITMAP bmp;
    BOOL bResult = FALSE;

    //code
    //Load The Bitmap As Image
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ImageResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if(hBitmap)
    {
        bResult = TRUE;
        
        //get bitmap structure from the loaded bitmap image
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        //Generate OpenGL Texture Object
        glGenTextures(1, Texture);  

        //bind to the newly Created texture object
        glBindTexture(GL_TEXTURE_2D, *Texture);

        //Unpack the image into memory for faster loading
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, (void*)bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return(bResult);
}

void printGLInfo(void)
{
    // variable declarations
    GLint numExtensions, i;

    // code
    // print opengl information
    fprintf(gpFile, "OpenGL Information!!\n");
    fprintf(gpFile, "********************\n");
    fprintf(gpFile, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "********************\n");

    // pp
    fprintf(gpFile, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // get number of extensions
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    // printf opengl extensions
    for (i = 0; i < numExtensions; i++)
    {
        fprintf(gpFile, " OpenGL Extension No = %d : %s \n", i, glGetStringi(GL_EXTENSIONS, i));
    }
}

void resize(int width, int height)
{
    // code

    // if heigth by accident is less than 0 than heigth should be 1
    if (height <= 0)
    {
        height = 1;
    }

    // set the viewport
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    PerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

//matrix stack
Stack* stack = new Stack();

void PushMatrix(const mat4 mat)
{
    stack->push(mat);
}

mat4 PopMatrix(void)
{
    return(stack->pop());
}

void solarDraw()
{
    //for texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture_SolarSystem);
    glUniform1i(textureSamplerUniform, 0);

    //bind with vao
    glBindVertexArray(gVao_Cube);
        // draw the vertex arrays
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0); // unbind with vao_Cube
}

void sunDraw()
{
    // *** bind vao ***
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture_Sun);
    glUniform1i(textureSamplerUniform, 0);
        
    glBindVertexArray(gVao_sphere);
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    // *** unbind vao ***
    glBindVertexArray(0);
}

void earthDraw()
{
    // *** bind vao ***
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture_Earth);
    glUniform1i(textureSamplerUniform, 0);
        
    glBindVertexArray(gVao_sphere);
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    // *** unbind vao ***
    glBindVertexArray(0);
}

void moonDraw()
{
    // *** bind vao ***
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture_Moon);
    glUniform1i(textureSamplerUniform, 0);
        
    glBindVertexArray(gVao_sphere);
    // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    // *** unbind vao ***
    glBindVertexArray(0);
}

void display(void)
{
    // code

    // clear OpenGL buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use shader program object
    glUseProgram(shaderProgramObject);

    //trasformations
    mat4 modelMatrix = mat4::identity(); // analogues to glloadidentity() in display 
    mat4 translationMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();
    mat4 ScaleMatrix = mat4::identity();

    mat4 ViewMatrix = mat4::identity();
    ViewMatrix = vmath::lookat(vec3(0.0f, 0.3f, 4.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
   
    // send above matrix to vertex shader in uniform
    glUniformMatrix4fv(ProjectionMatrixUniform, 1, GL_FALSE, PerspectiveProjectionMatrix);  
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, ViewMatrix);
   // GL_POSITION = mvpMatrixUniform * aPosition;
   
    PushMatrix(modelMatrix);
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    solarDraw();
    modelMatrix = PopMatrix();
//-----------------------------------------------

    PushMatrix(modelMatrix);
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    sunDraw();
    modelMatrix = PopMatrix();
//-----------------------------------------------

    PushMatrix(modelMatrix);
    
    rotationMatrix = vmath::rotate(fYear, 0.0f, 1.0f, 0.0f);  //y
    ScaleMatrix = vmath::scale(0.50f,0.50f,0.50f);
    translationMatrix = vmath::translate(2.5f, 0.0f, 0.0f);
    modelMatrix = ScaleMatrix * rotationMatrix * translationMatrix ;

    PushMatrix(modelMatrix);

    rotationMatrix =  mat4::identity();
    rotationMatrix = vmath::rotate(fDate, 0.0f, 0.0f, 1.0f);  //z
    modelMatrix = modelMatrix * rotationMatrix;
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    earthDraw();
    modelMatrix = PopMatrix();

//-----------------------------------------------
    ScaleMatrix = mat4::identity();
    ScaleMatrix = vmath::scale(0.40f,0.40f,0.40f);
    rotationMatrix =  mat4::identity();
    rotationMatrix = vmath::rotate(fMoon, 0.0f, 1.0f, 0.0f);  //y
    translationMatrix =  mat4::identity();
    translationMatrix = vmath::translate(1.8f, 0.0f, 0.0f);
    modelMatrix = modelMatrix *  ScaleMatrix * rotationMatrix * translationMatrix ;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    moonDraw();
    modelMatrix = PopMatrix();

    // unUse shader program object
    glUseProgram(0);

    // swap the buffer
    SwapBuffers(ghdc);
}

void update(void)
{
    // code
    
    if(fDate < 360.0f)
    {
        fDate = fDate + 0.04;
    }else
    {
        fDate = 0.0f;
    }

    if(fYear< 360.0f)
    {
        fYear = fYear + 0.08;
    }else
    {
        fYear = 0.0f;
    }

    if(fMoon < 360.0f)
    {
        fMoon = fMoon + 0.06;
    }else
    {
        fMoon = 0.0f;
    }
}

void uninitialise(void)
{
    // Local Function declarations
    void toggleFullScreen(void);

    // code

    // ifuser is exiting in full screen than restore the full screen back to normal
    if (gbFullScreen == TRUE)
    {
        toggleFullScreen();

        gbFullScreen = FALSE;
    }

    if(Texture_SolarSystem)
    {
        glDeleteTextures(1, &Texture_SolarSystem);
        Texture_SolarSystem = 0;
    }

    if(Texture_Earth)
    {
        glDeleteTextures(1, &Texture_Earth);
        Texture_Earth = 0;
    }

    if(Texture_Sun)
    {
        glDeleteTextures(1, &Texture_Sun);
        Texture_Sun = 0;
    }

    if(Texture_Moon)
    {
        glDeleteTextures(1, &Texture_Moon);
        Texture_Moon = 0;
    }

    // free vbo of elements
    if(gVbo_sphere_element)
    {
        glDeleteBuffers(1, &gVbo_sphere_element);
        gVbo_sphere_element = 0;
    }

    // free vbo of normals
    if(gVbo_sphere_texcoord)
    {
        glDeleteBuffers(1, &gVbo_sphere_texcoord);
        gVbo_sphere_texcoord = 0;
    }

    // free vbo of position
    if(gVbo_sphere_position)
    {
        glDeleteBuffers(1, &gVbo_sphere_position);
        gVbo_sphere_position = 0;
    }

    // free vao
    if(gVao_sphere)
    {
        glDeleteVertexArrays(1, &gVao_sphere);
        gVao_sphere = 0;
    }

    // ditach , delete shader objects , and shader program object generically for any number and any type of shader
    if (shaderProgramObject)
    {
        // 1) check the shader program object is still there
        glUseProgram(shaderProgramObject);

        // 2) get number of  shaders and continue only if number of shaders is grater than 0
        GLint numShaders = 0;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders > 0)
        {
            // 3) create a buffer / array to hold shader objects of obtained size
            GLuint *pShaders = (GLuint *)malloc(numShaders * sizeof(GLuint));
            if (pShaders != NULL)
            {
                // 4) get shader objects in this buffers
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);

                //5) start the loop for obtained number of shader and resize the group dettach and delete
                // every shader from the buffer / array
                for (GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i]);
                    pShaders[i] = 0;
                }
            }
            //6) free the buffer / array
            free(pShaders);
            pShaders = 0;
        }
        //7) delete the shader program object
        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
    }

    // make hdc as currnt context by relising rendering context as current context
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // delete the rendering contexr
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // Release the DC
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // Destroy window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Close the file
    if (gpFile)
    {
        fprintf(gpFile, "Program Terminated Successfully....\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
