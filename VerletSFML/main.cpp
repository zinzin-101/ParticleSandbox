#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include <windows.h>    // included for Windows Touch
#include <windowsx.h>   // included for point conversion
#define MAXPOINTS 50

#include "solver.hpp"
#include "renderer.hpp"
#include "utils/number_generator.hpp"
#include "utils/math.hpp"

const char g_szClassName[] = "myWindowClass";

std::stringstream ss;
sf::Text text[MAXPOINTS];
// You will use this array to track touch points
int points[MAXPOINTS][2];
int last_points[MAXPOINTS][2];
int diff_points[MAXPOINTS][2];
// You will use this array to switch the color / track ids
int idLookup[MAXPOINTS];


// This function is used to return an index given an ID
int GetContactIndex(int dwID) {
    for (int i = 0; i < MAXPOINTS; i++) {
        if (idLookup[i] == dwID) {
            return i;
        }
    }

    for (int i = 0; i < MAXPOINTS; i++) {
        if (idLookup[i] == -1) {
            idLookup[i] = dwID;
            return i;
        }
    }
    // Out of contacts
    return -1;
}

// Mark the specified index as initialized for new use
BOOL RemoveContactIndex(int index) {
    if (index >= 0 && index < MAXPOINTS) {
        idLookup[index] = -1;
        return true;
    }

    return false;
}

LRESULT OnTouch(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    BOOL bHandled = FALSE;
    UINT cInputs = LOWORD(wParam);
    PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
    POINT ptInput;
    if (pInputs) {
        if (GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT))) {
            for (UINT i = 0; i < cInputs; i++) {
                TOUCHINPUT ti = pInputs[i];
                int index = GetContactIndex(ti.dwID);
                if (ti.dwID != 0 && index < MAXPOINTS) {
                    // Do something with your touch input handle
                    ptInput.x = TOUCH_COORD_TO_PIXEL(ti.x);
                    ptInput.y = TOUCH_COORD_TO_PIXEL(ti.y);
                    ScreenToClient(hWnd, &ptInput);

                    if (ti.dwFlags & TOUCHEVENTF_UP) {
                        points[index][0] = -1;
                        points[index][1] = -1;
                        last_points[index][0] = -1;
                        last_points[index][1] = -1;
                        diff_points[index][0] = 0;
                        diff_points[index][1] = 0;

                        // Remove the old contact index to make it available for the new incremented dwID.
                        // On some touch devices, the dwID value is continuously incremented.
                        RemoveContactIndex(index);
                    }
                    else {
                        if (points[index][0] > 0) {
                            last_points[index][0] = points[index][0];
                            last_points[index][1] = points[index][1];
                        }
                       
                        points[index][0] = ptInput.x;
                        points[index][1] = ptInput.y;

                        if (last_points[index][0] > 0) {
                            diff_points[index][0] = points[index][0] - last_points[index][0];
                            diff_points[index][1] = points[index][1] - last_points[index][1];
                        }
                        

                    }
                }
            }
            bHandled = TRUE;
        }
        else {
            /* handle the error here */
        }
        delete[] pInputs;
    }
    else {
        /* handle the error here, probably out of memory */
    }
    if (bHandled) {
        // if you handled the message, close the touch input handle and return
        CloseTouchInputHandle((HTOUCHINPUT)lParam);
        return 0;
    }
    else {
        // if you didn't handle the message, let DefWindowProc handle it
        return DefWindowProc(hWnd, WM_TOUCH, wParam, lParam);
    }
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TOUCH:
        OnTouch(hwnd, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
    {
        //text.setString(L"Mouse down");
        /*char szFileName[MAX_PATH];
        HINSTANCE hInstance = GetModuleHandle(NULL);

        GetModuleFileName(hInstance, (LPSTR)szFileName, MAX_PATH);
        MessageBox(hwnd, (LPCSTR)szFileName, (LPCSTR)"This program is:", MB_OK | MB_ICONINFORMATION);*/
    }
    break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}



static sf::Color getRainbow(float t)
{
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * Math::PI);
    const float b = sin(t + 0.66f * 2.0f * Math::PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
}

static sf::Color getPinkBlue(float t)
{
    // blue: 5, 250, 246
    // pink: 255, 5, 213
    const float r = 255 + (5 - 255)*t;
    const float g = 5 + (250 - 5) * t;
    const float b = 213 + (246 - 213) * t;
    return { static_cast<uint8_t>(r),
            static_cast<uint8_t>(g),
            static_cast<uint8_t>(b) };
}

static Solver solver;

int unsigned objIndex = 0;
static void InstantiateObject(sf::Vector2f pos, TYPE type) {
    solver.addObject(pos, type);
    objIndex++;
}

static void InstantiateBrush(sf::Vector2f pos, TYPE type, float size) {
    float halfSize = size / 2.0f;
    for (float i = -halfSize; i <= halfSize; i += 3.0f) {
        for (float j = -halfSize; j <= halfSize; j += 3.0f) {
            sf::Vector2f temp = { i,j };
            InstantiateObject(pos + temp, type);
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = (LPCWSTR)g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);



    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        (LPCWSTR)g_szClassName,
        L"App name",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1000,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }


    // the following code initializes the points
    for (int i = 0; i < MAXPOINTS; i++) {
        points[i][0] = -1;
        points[i][1] = -1;
        last_points[i][0] = -1;
        last_points[i][1] = -1;
        diff_points[i][0] = 0;
        diff_points[i][1] = 0;
        idLookup[i] = -1;
    }



    DWORD Style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
    HWND  View1 = CreateWindowEx(WS_EX_CLIENTEDGE,
        (LPCWSTR)g_szClassName,
        NULL,
        Style,
        0, 0, 1500, 1000,
        hwnd, NULL, hInstance, NULL);

    // register the window for touch instead of gestures
    int touch_success = RegisterTouchWindow(View1, 0);
    // SFML setup

    // Create window
    constexpr int32_t window_width  = 1500;
    constexpr int32_t window_height = 1000;

    //sf::ContextSettings settings;
    //settings.antialiasingLevel = 1;
    //sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Simulation", sf::Style::Default, settings);

    sf::RenderWindow window(View1);
    window.create(View1);

    const uint32_t frame_rate = 60;
    window.setFramerateLimit(frame_rate);

    // setup font
    sf::Font font;
    if (!font.loadFromFile("THSarabunNew.ttf")) {/* error...*/ }
    //sf::Text text;
    //text.setFont(font); // font is a sf::Font
    //text.setString(L"สระบอล");
    //text.setCharacterSize(100); // in pixels, not points!
    //text.setFillColor(sf::Color::White);
    //text.setPosition(480, 50);


    for (int i = 0; i < MAXPOINTS; i++) {
        // select the font
        text[i].setFont(font); // font is a sf::Font

        // set the string to display
        ss << i;
        text[i].setString(ss.str());
        ss.str("");

        // set the character size
        text[i].setCharacterSize(64); // in pixels, not points!

        // set the color
        text[i].setFillColor(sf::Color::Green);
        //text.setPosition(20, 100);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    //Solver   solver;
    Renderer renderer{window};

    // Solver configuration, boundary constrain
    solver.setConstraint({static_cast<float>(window_width) * 0.5f, static_cast<float>(window_height) * 0.5f}, 450.0f);
    solver.setSubStepsCount(4);
    solver.setSimulationUpdateRate(frame_rate);

    // Set simulation attributes
    const float        object_spawn_delay    = 0.02f;
    const float        object_spawn_speed    = 1000.0f;
    const sf::Vector2f object_spawn_position = {80.0f, 150.0f};
    const float        object_min_radius     = 5.0f;
    const float        object_max_radius     = 8.0f;
    const uint32_t     max_objects_count     = 2500;
    const float        max_angle             = 1.0f;
    float colorIndex = 0;

    //create rope
    //int objIndex = 0;
    //sf::Vector2f startRopePos = {300,600};
    //solver.addObject(startRopePos, 5, true);
    //solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{0,0});
    //solver.m_objects[objIndex].color = sf::Color::White;//getPinkBlue(objIndex/30.0f);
    //objIndex++;
    //for (int i = 1; i < 40; i++) {
    //    solver.addObject(startRopePos + sf::Vector2f(i*15,0), 5, false);
    //    solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{ 0,0 });
    //    solver.m_objects[objIndex].color = sf::Color::White;// getPinkBlue(objIndex / 30.0f);
    //    Link& link1 = solver.addLink(objIndex - 1, objIndex);
    //    objIndex++;
    //}
    //solver.addObject(startRopePos + sf::Vector2f(40 * 15, 0), 5, true);
    //solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{ 0,0 });
    //solver.m_objects[objIndex].color = sf::Color::White;//getPinkBlue(objIndex / 30.0f);
    //Link& link1 = solver.addLink(objIndex - 1, objIndex);
    //objIndex++;

    /*sf::Vector2f startRopePos = { 500,500 };
    int startIndex = objIndex;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            solver.addObject(startRopePos + sf::Vector2f(i * 10, j * 10), 5, false);

            if (j > 0) {
                Link& link = solver.addLink(objIndex, objIndex - 1);
            }

            if (i > 0 && objIndex - 10 > startIndex) {
                Link& link = solver.addLink(objIndex - 10, objIndex);
            }

            objIndex++;
        }
    }

    objIndex++;*/

    srand(time(NULL));

    sf::Clock clock;
    unsigned int frameNum = 0;

    bool isLeftClick = false;
    bool isRightClick = false;
    TYPE selectedType = SAND;
    float brushSize = 5.0f;
    int holdLagFrame = 5;

    bool isLeftDown = false;
    bool isRightDown = false;
 
    Msg.message = ~WM_QUIT;
    while (Msg.message != WM_QUIT)
    {
        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        else {

            frameNum++;
            //if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() >= object_spawn_delay) {
            //    clock.restart();
            //    for (int i = 0; i < 5; i++) {
            //        VerletObject& object = solver.addObject(object_spawn_position + sf::Vector2f(0, i * object_max_radius * 2), RNGf::getRange(object_min_radius, object_max_radius), false);
            //        const float angle = 0.5f;//max_angle * sin(t) + Math::PI * 0.5f;
            //        solver.setObjectVelocity(object, object_spawn_speed * sf::Vector2f{ cos(angle), sin(angle) });
            //        object.color = getPinkBlue(colorIndex);
            //        std::cout << solver.getObjectsCount() << std::endl;
            //    }
            //    // inc color
            //    colorIndex += 0.005f;
            //    if (colorIndex >= 1.0f) colorIndex = 0.0f;
            //}
            solver.updateMousePos(sf::Mouse::getPosition(window));
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                    solver.deleteBrush(5.0f);
                }
                else if (selectedType == NONE) {
                    solver.applyMouseForce();
                }
                else if (frameNum % holdLagFrame == 0 || !isLeftClick) {
                    sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                    sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                    //InstantiateObject(mousePosF, selectedType);
                    InstantiateObject(mousePosF, selectedType);
                }

                isLeftClick = true;
            }
            else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                isLeftClick = false;
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                    solver.deleteBrush(brushSize);
                }
                else if (selectedType == NONE) {
                    solver.applyMouseForce();
                }
                else if (frameNum % holdLagFrame == 0 || !isRightClick) {
                    sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                    sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                    //InstantiateObject(mousePosF, selectedType);
                    InstantiateBrush(mousePosF, selectedType, brushSize);
                }

                isRightClick = true;
            }
            else if (!sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                isRightClick = false;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                selectedType = SAND;
                std::cout << "Current Type: Sand" << std::endl;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                selectedType = WATER;
                std::cout << "Current Type: Water" << std::endl;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                selectedType = CONCRETE;
                std::cout << "Current Type: Concrete" << std::endl;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                selectedType = LAVA;
                std::cout << "Current Type: Lava" << std::endl;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) {
                selectedType = GAS;
                std::cout << "Current Type: Gas" << std::endl;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) {
                selectedType = NONE;
                std::cout << "Current Type: None" << std::endl;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                brushSize += 0.5f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                brushSize -= 0.5f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && !isLeftDown) {
                holdLagFrame++;
                isLeftDown = true;
            }
            else {
                isLeftDown = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && !isRightDown) {
                holdLagFrame--;
                isRightDown = true;
            }
            else {
                isRightDown = false;
            }


            brushSize = abs(brushSize);

            if (holdLagFrame < 1) {
                holdLagFrame = 1;
            }

            solver.update();

            window.clear(sf::Color::White);
                renderer.render(solver);
                
                /*for (int i = 0; i < MAXPOINTS; i++) {
                    if (points[i][0] >= 0) {
                        text[i].setPosition(points[i][0], points[i][1]);
                        window.draw(text[i]);
                    }

                }*/

            window.display();

        }

        
    }

    return Msg.wParam;
}
