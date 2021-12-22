#include <iostream>
#include <utility>
#include <cassert>
#include <vector>
#include <string>
#include <chrono>
#include <map>

#include "Draw/ez-draw.h"
using namespace std;
using namespace std::chrono;

class SHAPE; // Initiate class SHAPE before SHAPES typedef so we don't get errors
typedef map <int, SHAPE*> SHAPES;

// Class SHAPE : Will either be a RECTANGLE or a HOLE
class SHAPE 
{
    public:
        int x1      = 0;
        int y1      = 0;
        int x2      = 0;
        int y2      = 0;
        int id      = 0;
        int origin  = 0;

        bool isRotated = false;

        // Colors for Draw memory
        Ez_uint32 Color;

    // ------------------------------------------------------------------------------------
    // FUNCTIONS
    // ------------------------------------------------------------------------------------

    void Print(string Type, int maxID, int maxDims, int maxCoords);

    // Rotate : Flips Shape1 (this) 90° : Shape1->Rotate();
    void Rotate();

    // sameAs : Shape1 (this) same properties as Shape2 (Shape) : Shape1->sameAs(Shape2);
    bool sameAs(SHAPE* Shape);

    // smallerThan : Shape1 (this) smaller than Shape2 (Shape) : Shape1->smallerThan(Shape2);
    bool smallerThan(SHAPE* Shape);

    // fitsIn : Shape1 (this) fits inside Shape2 (Shape) : Shape1->fitsIn(Shape2);
    bool fitsIn(SHAPE* Shape);

    // Overlaps : Checks if Shape1 (this) overlaps with Shape 2 (Shape) : Shape1->Overlaps(Shape2);
    bool Overlaps(SHAPE* Shape);

    // isCovered : Shape1 (this) covered by some Shape in Shapes : Shape1->isCovered(Shapes, ExceptionShape);
    bool isCovered(SHAPES* Shapes, SHAPE* ExceptionShape);

    // removeShape : Removes Shape1 (this) from Shapes : Shape1->Rotate(Shapes);
    void removeFrom(SHAPES* Shapes);

    // appendTo : Adds Shape1 to Shapes : Shape1->appendTo(Shapes);
    void appendTo(SHAPES* Shapes);
};

class RECTANGLE : public SHAPE {};
class HOLE : public SHAPE {};

// Result Class
class RESULT
{
    public:
        int Width = 0;
        int Height = 0;
        int Theoretical_Height = 0;
        int Rect_Area = 0;
        bool Rotate = false;
        int Holes_Created = 0;
        int Sort_Strategy = 1;

        duration <int64_t, nano> Total_Time;

        SHAPES Rectangles;
};

// GLOBAL Class
class GLOBAL
{
    public:
        int Rect_ID_Counter = 1;
        int Hole_ID_Counter = 1;

        vector<SHAPE*> Stored_Rectangles;
        SHAPES Rectangles;
        SHAPES Holes;

        // Run Info
        int Width = 0;
        int Height = 0;
        int Theoretical_Height = 0;
        int Rect_Area = 0;
        bool Rotate = false;
        int Sort_Strategy = 0;

        chrono::_V2::system_clock::time_point Start_Time;
        chrono::_V2::system_clock::time_point End_Time;
        duration <int64_t, nano> Total_Time;

    // ------------------------------------------------------------------------------------
    // GLOBAL FUNCTIONS
    // ------------------------------------------------------------------------------------

    // newRectID : Returns a new ID for new Rectangle;
    int newRectID();
    // newHoleID : Returns a new ID for new Hole;
    int newHoleID();
    // resetRectCounter : Resets the ID counter
    void resetRectCounter();
    // resetHoleCounter : Resets the ID counter
    void resetHoleCounter();

    // newRectangle : Adds a new Rectangle in Rectangles;
    void newRectangle(SHAPE* Rectangle);
    // newHole : Adds a new Hole in Holes;
    void newHole(SHAPE* Hole);
    
    // clearShapes : clears rectangles and holes
    void clearShapes();

    // Copy Shapes //
    SHAPES copyRectangles();
    SHAPES copyHoles();
    // ----------- //

    RESULT* getResult();
    void Solve(int Sort_Strategy);
};

// EZ Class
class EZ 
{
    public:
        Ez_window WINDOW;
        SHAPE* CANVAS = NULL;

        float DEFAULT_SCALE = 1;
        float RECT_SCALE = DEFAULT_SCALE;
        float ZOOM_INC = 0.05; // Increment in x,y fact. for zooming

        // Colors
        bool RAINBOW = false;
        float SATURATION = 78;  // /100
        float VALUE = 100; // /100

        bool SHOW_IDS = true;

        // Offsets for Controls
        int MARGIN = 20; // (Start Offset)

        int PAN_SPEED = 30;
        int VERTICAL = MARGIN;
        int HORIZONTAL = MARGIN;
        // ------- //
    
    // drawOutlined : Draw the border of a rectangle
    void drawOutlined(SHAPE* Rectangle);
    // drawFilled : Draw the whole rectangle (+ border)
    void drawFilled(SHAPE* Rectangle);
    // drawCanvas : Draw/Redraw Canvas with all Rectangles ( Update settings )
    void drawCanvas(RESULT* EndResult);
};