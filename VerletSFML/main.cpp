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
//sf::Text text[MAXPOINTS];
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
static VerletObject& InstantiateObject(sf::Vector2f pos, TYPE type) {
    VerletObject& obj = solver.addObject(pos, type);
    objIndex++;

    return obj;
}

static void InstantiateSpawner(sf::Vector2f pos, TYPE type, int delay, float radius) {
    VerletObject& spawner = solver.addObject(pos, SPAWNER);
    spawner.spawnerType = type;
    spawner.counter = delay;
    spawner.bounce = radius;
}

static void InstantiateSpawner(sf::Vector2f pos, TYPE type, float speed, float radius) {
    VerletObject& spawner = solver.addObject(pos, SPAWNER);
    spawner.spawnerType = type;
    spawner.frictionCoeff = speed;
    spawner.bounce = radius;
}

static void InstantiateBrush(sf::Vector2f pos, TYPE type, float size) {
    /*float halfSize = size / 2.0f;
    for (float i = -halfSize; i <= halfSize; i += 3.0f) {
        for (float j = -halfSize; j <= halfSize; j += 3.0f) {
            sf::Vector2f temp = { i,j };
            InstantiateObject(pos + temp, type);
        }
    }*/

    solver.addObjectCluster(pos, type, size);
}

bool isUpdatingString = false;
float minStringDist = 1.0f;
std::vector<sf::Vector2f> stringPosVec;

static void startString(sf::Vector2f pos, int frameDelay) {
    stringPosVec.push_back(pos);
    isUpdatingString = true;
    minStringDist = frameDelay * 10.0f;
}

static void updateString(Solver& solver, sf::Vector2f pos) {
    sf::Vector2f vec = pos - stringPosVec[stringPosVec.size() - 1];
    float dist = solver.getVectorMagnitude(vec);

    if (dist < minStringDist) {
        return;
    }

    sf::Vector2f normal = solver.getNormalizedVector(vec);
    sf::Vector2f nextPos = stringPosVec[stringPosVec.size()-1] + (normal * minStringDist);

    stringPosVec.push_back(nextPos);
}

static void endString(Solver& solver, float radius) {
    uint64_t size = stringPosVec.size();

    if (size < 2) {
        stringPosVec.clear();
        isUpdatingString = false;
        return;
    }

    sf::Vector2f vec = stringPosVec[size - 1] - stringPosVec[size - 2];
    sf::Vector2f normal = solver.getNormalizedVector(vec);
    float dist = solver.getVectorMagnitude(vec);

    sf::Vector2f finalVecPos = stringPosVec[size - 1] + (normal * (dist + radius));

    uint64_t objIndex = solver.getObjectsCount();
    VerletObject& firstObj = InstantiateObject(stringPosVec[0], STRING);
    objIndex++;
    firstObj.pinned = true;
    for (uint64_t i = 1; i < stringPosVec.size(); i++) {
        InstantiateObject(stringPosVec[i], STRING);
        solver.addLink(objIndex - 1, objIndex);

        objIndex++;
    }

    VerletObject& lastObj = InstantiateObject(finalVecPos, STRING);
    solver.addLink(objIndex - 1, objIndex);
    lastObj.radius = radius;
    lastObj.mass = radius * 6.0f;

    stringPosVec.clear();
    isUpdatingString = false;
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
        L"Particle Sandbox",
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
    //text.setString("ball");
    //text.setCharacterSize(100); // in pixels, not points!
    //text.setFillColor(sf::Color::White);
    //text.setPosition(480, 500);

    sf::Text typeText;
    typeText.setFont(font);
    typeText.setCharacterSize(50);
    typeText.setFillColor(sf::Color::White);
    typeText.setPosition(60, 30);

    sf::Text speedText;
    speedText.setFont(font);
    speedText.setCharacterSize(50);
    speedText.setFillColor(sf::Color::White);
    speedText.setPosition(60, 80);

    sf::Text spreadText;
    spreadText.setFont(font);
    spreadText.setCharacterSize(50);
    spreadText.setFillColor(sf::Color::White);
    spreadText.setPosition(60, 130);

    sf::Text modeText;
    modeText.setFont(font);
    modeText.setCharacterSize(50);
    modeText.setFillColor(sf::Color::White);
    modeText.setPosition(60, 180);

    sf::Text toggleSimText;
    toggleSimText.setFont(font);
    toggleSimText.setCharacterSize(50);
    toggleSimText.setFillColor(sf::Color::White);
    toggleSimText.setPosition(60, 230);


    //for (int i = 0; i < MAXPOINTS; i++) {
    //    // select the font
    //    text[i].setFont(font); // font is a sf::Font

    //    // set the string to display
    //    ss << i;
    //    text[i].setString(ss.str());
    //    ss.str("");

    //    // set the character size
    //    text[i].setCharacterSize(64); // in pixels, not points!

    //    // set the color
    //    text[i].setFillColor(sf::Color::Green);
    //    //text.setPosition(20, 100);
    //}

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
    bool isMidClick = false;
    TYPE selectedType = SAND;
    float brushSize = 5.0f;
    float speed = 1.2f;
    int holdLagFrame = (int)(6.0f / speed);


    bool isLeftDown = false;
    bool isRightDown = false;

    bool isDeleteMode = false;
    bool isDDown = false;

    bool isHDown = false;

    bool isPeriodDown = false;
    bool isCommaDown = false;

    bool isVDown = false;
    bool toggleSimulation = true;

    //bool isCDown = false;
    //bool stringMode = false;
 
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
            solver.updateFrameNum(frameNum);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (isDeleteMode) {
                    solver.deleteBrush(5.0f);
                }
                /*else if (stringMode) {
                    if (!isUpdatingString) {
                        startString(solver.getCurrentMousePosF(), holdLagFrame);
                    }
                    else {
                        updateString(solver, solver.getCurrentMousePosF());
                    }
                }*/
                else if (selectedType == NONE) {
                    solver.applyMouseForce();
                }
                else if (selectedType == BLACKHOLE) {
                    if (!isLeftClick) {
                        sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                        sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                        InstantiateSpawner(mousePosF, selectedType, speed, brushSize);
                    }
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
                /*if (stringMode && isUpdatingString) {
                    endString(solver, brushSize);
                }*/

                isLeftClick = false;
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                if (isDeleteMode) {
                    solver.deleteBrush(brushSize);
                }
                else if (selectedType == NONE) {
                    sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                    sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                    solver.applyCentripetalForce(mousePosF, brushSize, speed);
                    //solver.applyMouseForce();
                }
                else if (selectedType == BLACKHOLE) {
                    if (!isRightClick) {
                        sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                        sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                        InstantiateSpawner(mousePosF, selectedType, speed, brushSize);
                    }
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

            if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                if (!isMidClick) {
                    sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                    sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                    //InstantiateSpawner(mousePosF, selectedType, (selectedType == BLACKHOLE ? speed : holdLagFrame),
                        //(selectedType == BLACKHOLE ? brushSize : brushSize * 10.0f));

                    if (selectedType == BLACKHOLE) {
                        InstantiateSpawner(mousePosF, selectedType, speed, brushSize);
                    }
                    else {
                        InstantiateSpawner(mousePosF, selectedType, holdLagFrame, brushSize * 10.0f);
                    }
                }
                isMidClick = true;
            }
            else {
                isMidClick = false;
            }

            for (int i = 0; i < MAXPOINTS; i++) {
                if (points[i][0] >= 0 && points[i][1] >= 0) {
                    sf::Vector2f touchPoint = { (float)points[i][0], (float)points[i][1] };
                    if (isDeleteMode) {
                        solver.deleteBrush(brushSize, touchPoint);
                    }
                    else if (selectedType == NONE) {
                        solver.applyForce(touchPoint);
                        //solver.applyCentripetalForce(touchPoint, brushSize);
                    }
                    else if (selectedType == BLACKHOLE) {
                        sf::Vector2i mousePosInt = solver.getCurrentMousePos();
                        sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
                        InstantiateSpawner(mousePosF, selectedType, speed, brushSize);
                    }
                    else if (frameNum % holdLagFrame == 0) {
                        InstantiateObject(touchPoint, selectedType);
                    }
                }
            }
    
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                selectedType = SAND;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                selectedType = WATER;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                selectedType = CONCRETE;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                selectedType = LAVA;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) {
                selectedType = GAS;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) {
                selectedType = OBSIDIAN;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num7)) {
                selectedType = DIRT;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num8)) {
                selectedType = WOOD;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9)) {
                selectedType = FIRE;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0)) {
                selectedType = NONE;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                if (!isDDown) {
                    isDeleteMode = !isDeleteMode;
                }

                isDDown = true;
            }
            else {
                isDDown = false;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                brushSize += 1.0f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                brushSize -= 1.0f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                /*if (!isLeftDown) {
                    speed -= 0.1f;
                }*/
                speed -= 0.1f;

                isLeftDown = true;
            }
            else {
                isLeftDown = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                /*if (!isRightDown) {
                    speed += 0.1f;
                }*/
                speed += 0.1f;

                isRightDown = true;
            }
            else {
                isRightDown = false;
            }
            speed = (speed < 0.5f ? 0.5f : speed);
            holdLagFrame = (int)(6.0f / speed);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F5)) {
                solver.clearAll();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                if (isDeleteMode) {
                    solver.deleteObjectsOfType(selectedType);
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                if (isDeleteMode) {
                    solver.deleteObjectsOfType(SPAWNER);
                }
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                if (isDeleteMode) {
                    solver.deleteSpawnersOfType(selectedType);
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
                if (!isHDown) {
                    solver.clearHalf();
                }

                isHDown = true;
            }
            else {
                isHDown = false;
            }

            int nextType = selectedType;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma)) {
                if (!isCommaDown) {
                    nextType--;
                    if (nextType < 0) {
                        nextType = NUM_OF_TYPE - 1;
                    }

                    if (nextType == SPAWNER || nextType == STRING) {
                        nextType--;
                    }
                }
                isCommaDown = true;
            }
            else {
                isCommaDown = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period)) {
                if (!isPeriodDown) {
                    nextType++;
                    
                    if (nextType == SPAWNER || nextType == STRING) {
                        nextType++;
                    }
                }
                isPeriodDown = true;
            }
            else {
                isPeriodDown = false;
            }
            nextType = nextType % NUM_OF_TYPE;
            selectedType = (TYPE)nextType;

            brushSize = abs(brushSize);

            if (holdLagFrame < 1) {
                holdLagFrame = 1;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
                if (!isVDown) {
                    toggleSimulation = !toggleSimulation;
                }
                isVDown = true;
            }
            else {
                isVDown = false;
            }

            /*if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
                if (!isCDown) {
                    stringMode = !stringMode;
                }

                isCDown = true;
            }
            else {
                isCDown = false;
            }*/

            solver.update(toggleSimulation);

            window.clear(sf::Color::White);
            renderer.render(solver);
                
                /*for (int i = 0; i < MAXPOINTS; i++) {
                    if (points[i][0] >= 0) {
                        text[i].setPosition(points[i][0], points[i][1]);
                        window.draw(text[i]);
                    }

                }*/

            std::ostringstream ss;
            ss << "Type: " << typeString[selectedType];
            typeText.setString(ss.str());

            std::ostringstream ss2;
            ss2 << "Speed: " << speed;
            speedText.setString(ss2.str());

            std::ostringstream ss3;
            ss3 << "Spread: " << brushSize;
            spreadText.setString(ss3.str());

            std::ostringstream ss4;
            ss4 << "Mode: ";
            if (isDeleteMode) {
                ss4 << "Delete";
            }
            /*else if (stringMode) {
                ss4 << "String";
            }*/
            else {
                ss4 << "Spawn";
            }
            modeText.setString(ss4.str());

            std::ostringstream ss5;
            ss5 << "Simulation: ";
            if (toggleSimulation) {
                ss5 << "Running";
            }
            else {
                ss5 << "Paused";
            }
            toggleSimText.setString(ss5.str());

            window.draw(typeText);
            window.draw(speedText);
            window.draw(spreadText);
            window.draw(modeText);
            window.draw(toggleSimText);

            window.display();

        }

        
    }

    return Msg.wParam;
}
