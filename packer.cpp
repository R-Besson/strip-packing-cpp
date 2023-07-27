// + ----------------------------------- +
// |                                     |
// |       Author: Romain BESSON         |
// |                                     |
// |     Start Date: 16 / 12 / 2021      |
// |                                     |
// |    Publish Date: 22 / 12 / 2021     |
// |                                     |
// + ----------------------------------- +

#include <algorithm>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "packer.h"
#include <cassert>
#include <cstdlib>
#include <conio.h>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>
#include <map>

using namespace std;
using namespace std::chrono;

// ------------------------------------------------------------------------------------
// GLOBAL DATA
// ------------------------------------------------------------------------------------
#define MAX_NB_RECTANGLES 100000
#define INT_INFINITY 100000000

GLOBAL Manager;
EZ Draw;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
HINSTANCE* hInstance = new HINSTANCE();

// ------------------------------------------------------------------------------------
// ~~ FUNCTIONS
// ------------------------------------------------------------------------------------

// adjustString : Fit string inside n of blanks
string adjustString(string s, int fit)
{
    string n = s;
    fit = (fit < s.length()) ? s.length() : fit;
    for (int i = 0; i < fit-s.length(); i++) {n += " ";}
    return n;
}

char* toCharStar(string s)
{
    return ((char *) s.c_str());
}
// ------------------------------------------------------------------------------------
// GLOBAL CLASS FUNCTIONS
// ------------------------------------------------------------------------------------

// newRectID : Returns a new ID for new Rectangle
int GLOBAL::newRectID()
{
    return this->Rect_ID_Counter++;
}
// newHoleID : Returns a new ID for new Hole
int GLOBAL::newHoleID()
{
    return this->Hole_ID_Counter++;
}
// resetRectCounter
void GLOBAL::resetRectCounter()
{
    this->Rect_ID_Counter = 0;
}
// resetHoleCounter
void GLOBAL::resetHoleCounter()
{
    this->Hole_ID_Counter = 0;
}
// newHole : Add a new Hole to Holes;
void GLOBAL::newHole(SHAPE* Hole)
{
    Hole->id = this->newHoleID();
    (this->Holes)[Hole->id] = Hole;
}
// newRectangle : Add a new Rectangle to Rectangles
void GLOBAL::newRectangle(SHAPE* Rectangle)
{
    (this->Rectangles)[this->newRectID()] = Rectangle;
}
// clearShapes : clears rectangles and holes
void GLOBAL::clearShapes()
{
    (this->Rectangles).clear();
    (this->Holes).clear();
}
// Copy Shapes //
SHAPES GLOBAL::copyRectangles()
{
    SHAPES Copied_Shapes;

    SHAPES::iterator it;
    for (it = this->Rectangles.begin(); it != this->Rectangles.end(); it++)
    {
        SHAPE* Rectangle = it->second;

        SHAPE* Copied = new SHAPE;

        Copied->x1 = Rectangle->x1;
        Copied->y1 = Rectangle->y1;
        Copied->x2 = Rectangle->x2;
        Copied->y2 = Rectangle->y2;

        Copied->id = Rectangle->id;
        Copied->origin = Rectangle->origin;

        Copied->appendTo(&Copied_Shapes);
    }
    return Copied_Shapes;
}
SHAPES GLOBAL::copyHoles()
{
    SHAPES Copied_Shapes;

    SHAPES::iterator it;
    for (it = this->Holes.begin(); it != this->Holes.end(); it++)
    {
        SHAPE* Hole = it->second;

        SHAPE* Copied = new SHAPE;
        
        Copied->x1 = Hole->x1;
        Copied->y1 = Hole->y1;
        Copied->x2 = Hole->x2;
        Copied->y2 = Hole->y2;

        Copied->id = Hole->id;
        Copied->origin = Hole->origin;

        Copied->appendTo(&Copied_Shapes);
    }
    return Copied_Shapes;
}
// ----------- //

// Get Run Result
RESULT* GLOBAL::getResult()
{
    RESULT* CurrentRun = new RESULT();

    CurrentRun->Width = this->Width;
    CurrentRun->Theoretical_Height = this->Theoretical_Height;
    CurrentRun->Rect_Area = this->Rect_Area;
    CurrentRun->Rotate = this->Rotate;
    CurrentRun->Sort_Strategy = this->Sort_Strategy;
    CurrentRun->Total_Time = this->Total_Time;
    CurrentRun->Holes_Created = this->Hole_ID_Counter;
    CurrentRun->Sort_Strategy = this->Sort_Strategy;
    CurrentRun->Rectangles = this->copyRectangles();

    for (SHAPES::iterator it = (CurrentRun->Rectangles).begin(); it != (CurrentRun->Rectangles).end(); it++)
    {
        SHAPE* Rectangle = it->second;

        // End Height
        if (Rectangle->y2 > CurrentRun->Height) CurrentRun->Height = Rectangle->y2;
    }

    return CurrentRun;
}

// ------------------------------------------------------------------------------------
// CLASS DEFINED FUNCTIONS
// ------------------------------------------------------------------------------------

void SHAPE::Print(string Type, int maxID, int maxDims, int maxCoords)
{
    cout << Type << " ID: " << adjustString(to_string(this->id)+",", maxID);

    cout << " Width: " << adjustString(to_string(this->x2 - this->x1)+",", maxDims);
    cout << " Height: " << adjustString(to_string(this->y2 - this->y1)+",", maxDims);

    cout << " [ X1: " << adjustString(to_string(this->x1)+",", maxCoords);
    cout << "Y1: " << adjustString(to_string(this->y1)+",", maxCoords);
    cout << "X2: " << adjustString(to_string(this->x2)+",", maxCoords);
    cout << "Y2: " << adjustString(to_string(this->y2), maxCoords-2) << " ], ";

    cout << "Origin: " << adjustString(to_string(this->origin), 4) << endl;
}
// Rotate : Flips Shape1 (this) 90° : Shape1->Rotate();
void SHAPE::Rotate()
{
    int oldX2 = this->x2;

    this->x2 = this->y2;
    this->y2 = oldX2;

    this->isRotated = !(this->isRotated);
}
// sameAs : Shape1 (this) same properties as Shape2 (Shape) : Shape1->sameAs(Shape2);
bool SHAPE::sameAs(SHAPE* Shape)
{
    if (
        this->x1 == Shape->x1 &&
        this->x2 == Shape->x2 &&
        this->y1 == Shape->y1 &&
        this->y2 == Shape->y2 &&
        this->id == Shape->id &&
        this->origin == Shape->origin)
        return true;
    return false;
}
// smallerThan : Shape1 (this) smaller than Shape2 (Shape) : Shape1->smallerThan(Shape2);
bool SHAPE::smallerThan(SHAPE* Shape)
{
    // Check if this shape fits in shape2
    // ONLY by comparing their Widths and Heights AND
    // Not their actual 2D positions

    if (
        (this->x2 - this->x1) < (Shape->x2 - Shape->x1) && // Width 1 smaller than Width 2
        (this->y2 - this->y1) < (Shape->y2 - Shape->y1)    // Height 1 smaller than Height 2
    )
        return true;
    return false;
}
// fitsIn : Shape1 (this) fits inside Shape2 (Shape) : Shape1->fitsIn(Shape2);
bool SHAPE::fitsIn(SHAPE* Shape)
{
    // Check if this shape fits in the other
    // By comparing all of the corners of this shape

    if (
        this->x1 >= Shape->x1 && // Top Left Corner of this shape is inside the other
        this->y1 >= Shape->y1 && //

        this->x2 <= Shape->x2 && // Bottom Right of this shape is inside the other
        this->y2 <= Shape->y2    // Therefore, a boundary is formed for this shape only using 2 corners of the other
    )
        return true;
    return false;
}
// Overlaps : Checks if Shape1 (this) overlaps with Shape 2 (Shape) : Shape1->Overlaps(Shape2);
bool SHAPE::Overlaps(SHAPE* Shape)
{
    // Check corners //
        // Check top left corner
        if (
            (this->x1 > Shape->x1) &&
            (this->x1 < Shape->x2) &&
            (this->y1 > Shape->y1) &&
            (this->y1 < Shape->y2)
        ) return true;

        // Check top right corner
        if (
            (this->x2 > Shape->x1) &&
            (this->x2 < Shape->x2) &&
            (this->y1 > Shape->y1) &&
            (this->y1 < Shape->y2)
        ) return true;

        // Check bottom left corner
        if (
            (this->x1 > Shape->x1) &&
            (this->x1 < Shape->x2) &&
            (this->y2 > Shape->y1) &&
            (this->y2 < Shape->y2)
        ) return true;

        // Check bottom right corner
        if (
            (this->x2 > Shape->x1) &&
            (this->x2 < Shape->x2) &&
            (this->y2 > Shape->y1) &&
            (this->y2 < Shape->y2)
        ) return true;
    // ------------- //


    // Check Edges don't overlap //
        // Check if the top horizontal edge of the rectangle passes through the hole
        if (
            (this->x1 < Shape->x2) &&
            (this->x2 > Shape->x1) &&
            (this->y1 >= Shape->y1) &&
            (this->y1 < Shape->y2)
        ) return true;

        // Check if the bottom horizontal edge of the rectangle passes through the hole
        if (
            (this->x1 < Shape->x2) &&
            (this->x2 > Shape->x1) &&
            (this->y2 <= Shape->y2) &&
            (this->y2 > Shape->y1)
        ) return true;

        // Check if the left vertical edge of the rectangle passes through the hole
        if (
            (this->y1 < Shape->y2) &&
            (this->y2 > Shape->y1) &&
            (this->x1 < Shape->x2) &&
            (this->x1 >= Shape->x1)
        ) return true;

        // Check if the right vertical edge of the rectangle passes through the hole
        if (
            (this->y1 < Shape->y2) &&
            (this->y2 > Shape->y1) &&
            (this->x2 <= Shape->x2) &&
            (this->x2 > Shape->x1)
        ) return true;
    // ------------------------- //

    return false; // No overlap detected
}
// isCovered : Shape1 (this) covered by some Shape in Shapes : Shape1->isCovered(Shapes, ExceptionShape);
bool SHAPE::isCovered(SHAPES* Shapes)
{
    // Iterates through all the shapes and checks if it is covered by a Shape AND
    // We also pass an argument 'ExceptionShape' if we are for example: Breaking a Shape into new Shapes

    SHAPES::iterator it;

    for (it = (*Shapes).begin(); it != (*Shapes).end(); it++)
    {
        SHAPE* shape = it->second;
        if (this->fitsIn(shape)) return true;
    }
    return false;
}
// removeFrom : Removes Shape1 (this) from Shapes : Shape1->Rotate(Shapes);
void SHAPE::removeFrom(SHAPES* Shapes)
{
    (*Shapes).erase(this->id);
}
// appendTo : Adds Shape1 to Shapes : Shape1->appendTo(Shapes);
void SHAPE::appendTo(SHAPES* Shapes)
{
    (*Shapes)[this->id] = this;
}

// ------------------------------------------------------------------------------------
// FUNCTIONS
// ------------------------------------------------------------------------------------

// clearConsole : Clears the Windows console
void clearConsole()
{
    system("CLS");
}
// setColor : Set color for Cout -> Windows Console
void setColor(int Color)
{
    SetConsoleTextAttribute(hConsole, Color);
}
// resetColor : Set color to default for Cout -> Windows Console
void resetColor()
{
    SetConsoleTextAttribute(hConsole, 7);
}
// checkSolve : Check Solving Possibility
int checkSolve()
{
    for (vector<SHAPE*>::iterator it = Manager.Stored_Rectangles.begin(); it != Manager.Stored_Rectangles.end(); it++)
    {
        SHAPE* Rectangle = *it;

        if ( // (1) Check if Rectangle is placable without rotations
            !Manager.Rotate && 
            (Rectangle->x2 - Rectangle->x1) > Manager.Width
        ) return Rectangle->id;

        if ( // (2) Check if Rectangle is placable even with rotations
            (Rectangle->x2 - Rectangle->x1) > Manager.Width && 
            (Rectangle->y2 - Rectangle->y1) > Manager.Width
        ) return Rectangle->id;
    }
    return 0;
}
// printUsage : Prints Usage Info to user
void printUsage()
{
    cout << "   ./packer <rectangle_list> <width> <can_rotate> <sort_strategy>\n\n";
    cout << "Examples:\n";
    cout << "   ./packer list 100           --> width => 100\n";
    cout << "   ./packer list ??? 0         --> non-rotatable solution\n";
    cout << "   ./packer list ??? 1         --> rotatable solution\n";
    cout << "   ./packer list ??? ? 1       --> Sort Strategy => Area\n";
    cout << "   ./packer list ??? ? 2       --> Sort Strategy => Height\n";
    cout << "   ./packer list ??? ? 3       --> Sort Strategy => Automatic\n";
}
// printHelp : Prints Usage Info to user
void printHelp()
{
    cout << "\nUsage:\n";
    printUsage();
}
// printError : Prints an Error to user
void printError(string message, int type, SHAPES* FaultyShapes)
{
    setColor(12);
    cout << "\nFailed:\n------------------------------------------------------------\n";
    resetColor();
    switch (type) {
        case 1:
            cout << "Error Message: " << message;
            cout << "\n\nCorrect Usage:\n";
            printUsage();

            break;
        case 2:
            cout << "Error Message: " << message;
            cout << "\n\nFile Format:\n";
            cout << "   Rectangle <width, height>\n\n";
            cout << "Example (example.txt):\n";
            cout << "   10 20           --> New Rectangle with 10 Width, 20 Height\n";
            cout << "   20 30           --> New Rectangle with 20 Width, 30 Height\n";
            cout << "   30 40           --> New Rectangle with 30 Width, 40 Height\n";

            break;
        case 3:
            cout << "Error Message: " << message << endl;

            break;
        case 4:
            SHAPES::iterator it;

            int bigID = 0;
            int bigDim = 0;
            int bigCoord = 0;

            for (it = FaultyShapes->begin(); it != FaultyShapes->end(); it++)
            {
                int key = it->first;
                SHAPE* Shape = it->second;

                // assert(key == Shape->id); // The key is normally the Hole's id

                //
                if (to_string(Shape->id).length() > bigID) bigID = to_string(Shape->id).length();

                int Dimensions = max((Shape->x2-Shape->x1), (Shape->y2-Shape->y1));
                if (to_string(Dimensions).length() > bigDim) bigDim = to_string(Dimensions).length();

                int Coords = max(Shape->x1,max(Shape->x2,max(Shape->y1,Shape->y2)));
                if (to_string(Coords).length() > bigCoord) bigCoord = to_string(Coords).length();
                //
            }

            for (it = FaultyShapes->begin(); it != FaultyShapes->end(); it++)
            {
                int key = it->first;
                SHAPE* Shape = it->second;

                // assert(key == Shape->id); // The key is normally the Shape's id

                Shape->Print((Shape->id > Manager.Rect_ID_Counter) ? "Hole:      " : "Rectangle: ", bigID+3, bigDim+3, bigCoord+3);
            }
    }
    setColor(12);
    cout << "------------------------------------------------------------\n";
    resetColor();
}

// awaitInput //
void ExitStartInput()
{
    int key = getch();

    switch (key) {
        case 115: return;
        case 113: exit(0);
    }

    ExitStartInput();
}
void awaitInput(string Message)
{
    cout << "\n" << Message << "...\n";
    ExitStartInput();
}
// ---------- //

// printRunInf : Print current run info
void printRunInf()
{
    cout << "\nYou are about to start packing:\n";
    cout << "\n--> Rectangles: " + to_string(Manager.Stored_Rectangles.size());
    cout << "\n--> Width: " + to_string(Manager.Width);
    cout << "\n--> Rotations: " << ((Manager.Rotate) ? "Yes\n" : "No\n");

    cout << "--> Sort Strategy: ";
    switch (Manager.Sort_Strategy)
    {
        case 1: cout << "By Area\n"; break;
        case 2: cout << "By Height\n"; break;

        default: cout << " By Area\n"; break;
    }

    awaitInput("Press 's' to start packing, or 'q' to abort");
}
// printEndInf : Prints the Ending Info
void printEndInf(RESULT* EndResult)
{
    resetColor();
    cout << "Theoretical Min. Height: "; setColor(10); cout << EndResult->Theoretical_Height << endl; resetColor();
    cout << "Height Calculated: "; setColor(10); cout << EndResult->Height << endl; resetColor();
    cout << "Holes Created: "; setColor(10); cout << EndResult->Holes_Created << endl; resetColor();
    cout << "Total Area: "; setColor(10); cout << EndResult->Rect_Area << endl; resetColor();
    
    cout << "Time Taken: "; setColor(10);
    if (duration <double, milli> (EndResult->Total_Time).count() < 10)
    {
        cout << duration <double, micro> (EndResult->Total_Time).count() << " micro s" << endl << endl; resetColor();
    } else cout << duration <double, milli> (EndResult->Total_Time).count() << " ms" << endl << endl; resetColor();

    float num = (EndResult->Height - EndResult->Theoretical_Height) * EndResult->Width * 100;
    float denum = EndResult->Height * EndResult->Width;
    float lost = (num / denum);
    float used = 100 - lost;
    
    cout << "Used % (green): " ; setColor(10); cout << used << " %" << endl; resetColor();
    cout << "Lost % (green): "; setColor(10); cout << lost << " %"  << endl;; resetColor();
    cout << "Sort Chosen: "; setColor(10); cout << ((EndResult->Sort_Strategy == 1) ? "By Area" : "By Height") << endl;; resetColor();
}
// byArea : Comparison function for Area
bool byArea(SHAPE* a, SHAPE* b)
{
    return (a->x2 * a->y2) > (b->x2 * b->y2);
}
// byHeight : Comparison function for height
bool byHeight(SHAPE* a, SHAPE* b)
{
    int heightA = a->y2 - a->y1;
    int heightRotatedA = a->x2 - a->x1;

    int heightB = b->y2 - b->y1;
    int heightRotatedB = b->x2 - b->x1;

    bool result = heightA < heightB;
    bool resultRotated = min(heightA, heightRotatedA) < min(heightB, heightRotatedB);

    return (Manager.Rotate) ? resultRotated : result;
}
// readRectangles : reads specified file path and sets Manager.Rectangles
bool readRectangles(char* fileName)
{
    FILE *at;

    if ((at = fopen(fileName, "r")) == NULL) return false;

    Manager.clearShapes();

    int w   = 0;
    int h   = 0;

    while (fscanf(at, "%d %d", &w, &h) != EOF)
    {
        RECTANGLE* NewRectangle = new RECTANGLE();

        NewRectangle->x2 = w;
        NewRectangle->y2 = h;
        NewRectangle->id = Manager.newRectID();

        Manager.Stored_Rectangles.push_back(NewRectangle);
    }

    return true;
}
// init_G_ : Initiate Global Variables
bool init_G_(int argc, char* argv[])
{
    if (((string) argv[1]) == "-help")
    {
        printHelp();
        return false;
    }
    // Check that we have right arguments //
    if ((argc < 4) || (argc > 5))
    {
        printHelp();
        return false; // Failed
    }
    // ---------------------------------- //

    // SET Global Variables //
    Manager.Width = atoi((char*) argv[2]);
    if (Manager.Width <= 0)
    {
        printError("Width cannot be 0", 1, NULL);
        return false; // Failed
    }
    Manager.Rotate = (argc > 3) ? atoi((char*) argv[3]) : false;
    Manager.Sort_Strategy = (atoi((char*) argv[4]) == 0) ? 1 : atoi((char*) argv[4]);
    Manager.Sort_Strategy = (Manager.Sort_Strategy > 3) ? 3 : ((Manager.Sort_Strategy < 1) ? 1 : Manager.Sort_Strategy);
    // -------------------- //

    // Specified File is Un-Readable or Doesn't Exist //
    if (!readRectangles(argv[1]))
    {
        printError("Failed to read file: " + (string) argv[1], 2, NULL);
        return false; // Failed
    }
    // ---------------------------------------------- //

    // Cap Input Rectangles Amount for Security (Changable limit) //
    if (Manager.Stored_Rectangles.size() > MAX_NB_RECTANGLES)
    {
        printError("Too many rectangles, MAX_NB_RECTANGLES => " + to_string(MAX_NB_RECTANGLES), 2, NULL);
        return false; // Failed
    }
    // ---------------------------------------------------------- //

    // Check Solving Possibility //
    int ID_Failed_RECT = checkSolve();  // Check that rectangle's size's match with specified Manager.Width
    if (ID_Failed_RECT)                 //
    {
        printError("Rectangle " + to_string(ID_Failed_RECT) + " is too big in relation to specified Width", 2, NULL);
        return false; // Failed
    }
    // ------------------------- //

    // Calculate Theoretical Min. Height + Total Rectangles AREA //            
    for (vector<SHAPE*>::iterator it = Manager.Stored_Rectangles.begin(); it != Manager.Stored_Rectangles.end(); it++)
    {
        SHAPE* Rectangle = *it;

        // T. Min. Height
        Manager.Theoretical_Height += (Rectangle->x2 - Rectangle->x1) * (Rectangle->y2 - Rectangle->y1);

        // Rect Area
        Manager.Rect_Area += (Rectangle->x2 - Rectangle->x1) * (Rectangle->y2 - Rectangle->y1);
    }
    Manager.Theoretical_Height /= Manager.Width;
    // -------------------------------- //

    return true; // Passed initiations
}

// printShapes //
void printRectangles()
{
    cout << "Rectangles:" << endl;
    SHAPES::iterator it;

    int bigID = 0;
    int bigDim = 0;
    int bigCoord = 0;

    for (it = Manager.Rectangles.begin(); it != Manager.Rectangles.end(); it++)
    {
        int key = it->first;
        SHAPE* Rectangle = it->second;

        // assert(key == Rectangle->id); // The key is normally the Hole's id

        //
        if (to_string(Rectangle->id).length() > bigID) bigID = to_string(Rectangle->id).length();

        int Dimensions = max((Rectangle->x2-Rectangle->x1), (Rectangle->y2-Rectangle->y1));
        if (to_string(Dimensions).length() > bigDim) bigDim = to_string(Dimensions).length();

        int Coords = max(Rectangle->x1,max(Rectangle->x2,max(Rectangle->y1,Rectangle->y2)));
        if (to_string(Coords).length() > bigCoord) bigCoord = to_string(Coords).length();
        //
    }
            
    for (it = Manager.Rectangles.begin(); it != Manager.Rectangles.end(); it++)
    {
        (it->second)->Print("", bigID+3, bigDim+3, bigCoord+3);
    }
}
void printHoles()
{
    cout << "Holes:" << endl;
    SHAPES::iterator it;
            
    for (it = Manager.Holes.begin(); it != Manager.Holes.end(); it++)
    {
        int H_key = it->first;
        SHAPE* Hole = it->second;

        // assert(H_key == Hole->id); // The key is normally the Hole's id

        cout << "ID: " << Hole->id << " x1: " << Hole->x1 << " x2: " << Hole->x2 << " y1: " << Hole->y1 << " y2: " << Hole->y2 << endl;
    }
}
// ----------- //

// ------------------------------------------------------------------------------------
// PACKING FUNCTIONS
// ------------------------------------------------------------------------------------

// calculateHeight : Returns the height of the Hole + the Rectangle
int calculateHeight(SHAPE* Rectangle, SHAPE* Hole)
{
    return Hole->y1 + (Rectangle->y2 - Rectangle->y1);
}
// getBestHole : Get the best hole to Cut for Packing using different methods
SHAPE* getBestHole(SHAPE* Rectangle, int Method)
{
    SHAPE* bestHoleFound = NULL;
    bool doRotation = false;

    switch (Method)
    {
        case 1: // [METHOD 1]: Choose hole that makes canvas height the smallest
            {
                double lowestHeightFound = INT_INFINITY;

                SHAPES::iterator it;
                
                for (it = Manager.Holes.begin(); it != Manager.Holes.end(); it++)
                {
                    int key = it->first;
                    SHAPE* Hole = it->second;

                    // Hole->Print("Considered HOLE ->");

                    // assert(key == Hole->id); // The key is normally the Hole's id

                    // Check Normal Rotation //
                    bool normalFits = Rectangle->smallerThan(Hole);
                    int normalH     = calculateHeight(Rectangle, Hole);

                    if (normalFits && normalH < lowestHeightFound) { // Check that Height increase is worthy of this Hole
                        lowestHeightFound = normalH;
                        bestHoleFound = Hole;
                        doRotation = false;
                    }
                    // --------------------- //

                    // Check Rotated Solution //
                    if (!Manager.Rotate) continue;

                    Rectangle->Rotate(); // Inner

                    bool rotationFits = Rectangle->smallerThan(Hole);
                    int rotationH     = calculateHeight(Rectangle, Hole);

                    if (rotationFits && rotationH < lowestHeightFound) { // Check that Rotated Height is worthy of the rectangle being rotated
                        lowestHeightFound = rotationH;
                        bestHoleFound = Hole;
                        doRotation = true;
                    }

                    Rectangle->Rotate(); // Outer
                    // ---------------------- //
                }
                break;
            }
    }

    if (doRotation) Rectangle->Rotate();

    return bestHoleFound;
}
// createNewHoles : Creates/Cuts Hole into new Holes based on a Shape
bool createNewHoles(SHAPE* Rectangle, SHAPE* Hole, SHAPES* Holes)
{
    // Create new holes from a Placement / Overlapping
    // ⬛ => Hole
    // ⬜ => Rectangle
    // So we enumerate all the 15 cases where the Rectangle can clip inside the Hole


    bool caseMet = false;

    SHAPE* newHole1 = NULL;
    SHAPE* newHole2 = NULL;
    SHAPE* newHole3 = NULL;

    // ⬜⬜⬜
    // ⬜⬜⬜
    // ⬜⬜⬜
    // [CASE 1]: Perfect fit!
    if (
        !caseMet &&
        Rectangle->x1 == Hole->x1 && // Rectangle's Left Vertical Edge is on the hole's Left Vertical Edge
        Rectangle->y1 == Hole->y1 && // Rectangle's Top Horizontal Edge is on the hole's Top Horizontal Edge
        Rectangle->x2 == Hole->x2 && // Rectangle's Right Vertical Edge is on the hole's Right Vertical Edge
        Rectangle->y2 == Hole->y2    // Rectangle's Bottom Horizontal Edge is on the hole's Bottom Horizontal Edge
    ) caseMet = true; // Only case where we add 0 Holes
    //

    // ⬜⬜⬜
    // ⬛⬛⬛
    // ⬛⬛⬛
    // [CASE 2]: Top Bar Horizontal
    if (
        !caseMet &&
        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge is right to the hole AND

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge passes through the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Rectangle->y2;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 2;
    }
    //

    // ⬛⬛⬜
    // ⬛⬛⬜
    // ⬛⬛⬜
    // [CASE 3]: Right Bar Vertical
    if (
        !caseMet &&
        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 < Hole->x2 && // 

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge is right to the hole AND

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Rectangle->x1;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 3;
    }
    //

    // ⬛⬛⬛
    // ⬛⬛⬛
    // ⬜⬜⬜
    // [CASE 4]: Bottom Bar Horizontal
    if (
        !caseMet &&
        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge is right to the hole AND

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 4;
    }
    //

    // ⬜⬛⬛
    // ⬜⬛⬛
    // ⬜⬛⬛
    // [CASE 5]: Left Bar Vertical
    if (
        !caseMet &&
        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge passes through the hole AND
        Rectangle->x2 < Hole->x2 && //

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Rectangle->x2;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 5;
    }
    //

    // ⬜⬛⬛
    // ⬛⬛⬛
    // ⬛⬛⬛
    // [CASE 6]: Top Left Corner
    if (
        !caseMet &&
        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge passes through the hole AND
        Rectangle->x2 < Hole->x2 && // 

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge passes through the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Rectangle->y2;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 6;

        newHole2 = new HOLE();
        newHole2->x1 = Rectangle->x2;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 6;
    }
    //

    // ⬛⬛⬜
    // ⬛⬛⬛
    // ⬛⬛⬛
    // [CASE 7]: Top Right Corner
    if (
        !caseMet &&
        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge Crosses in the hole AND
        Rectangle->x1 < Hole->x2 && // 

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge is right to the hole AND
        Rectangle->x2 >= Hole->x2 && // 

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge passes through the Hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Rectangle->x1;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 7;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Rectangle->y2;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 7;
    }
    //

    // ⬛⬛⬛
    // ⬛⬛⬛
    // ⬛⬛⬜
    // [CASE 8]: Bottom Right Corner
    if (
        !caseMet &&

        Rectangle->x1 < Hole->x2 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 > Hole->x1 && //

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge is right to the hole AND

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 8;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Rectangle->x1;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 8;
    }

    // ⬛⬛⬛
    // ⬛⬛⬛
    // ⬜⬛⬛
    // [CASE 9]: Bottom Left Corner
    if (
        !caseMet &&

        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge passes through the hole AND 
        Rectangle->x2 < Hole->x2 && // 

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge passes through the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 9;

        newHole2 = new HOLE();
        newHole2->x1 = Rectangle->x2;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 9;
    }

    // ⬛⬜⬛
    // ⬛⬜⬛
    // ⬛⬜⬛
    // [CASE 10]: Middle Vertical Bar
    if (
        !caseMet &&

        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 < Hole->x2 && //

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge passes through the hole AND
        Rectangle->x2 < Hole->x2 && //

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Rectangle->x1;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 10;

        newHole2 = new HOLE();
        newHole2->x1 = Rectangle->x2;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 10;
    }

    // ⬛⬛⬛
    // ⬜⬜⬜
    // ⬛⬛⬛
    // [CASE 11]: Middle Horizontal Bar
    if (
        !caseMet &&

        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge if right to the hole AND

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge is below the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 11;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Rectangle->y2;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 11;
    }

    // ⬛⬜⬛
    // ⬛⬛⬛
    // ⬛⬛⬛
    // [CASE 12]: Top Rectangle Pop-Out
    if (
        !caseMet &&

        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 < Hole->x2 && //

        Rectangle->y1 <= Hole->y1 && // Rectangle's Top Horizontal Edge is above the hole AND

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge pases through the hole AND
        Rectangle->x2 < Hole->x2 && //

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge passes through the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Rectangle->x1;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 12;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Rectangle->y2;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 12;

        newHole3 = new HOLE();
        newHole3->x1 = Rectangle->x2;
        newHole3->y1 = Hole->y1;
        newHole3->x2 = Hole->x2;
        newHole3->y2 = Hole->y2;
        newHole3->origin = 12;
    }

    // ⬛⬛⬛
    // ⬛⬛⬜
    // ⬛⬛⬛
    // [CASE 13]: Right Rectangle Pop-Out
    if (
        !caseMet &&

        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 < Hole->x2 && //

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 >= Hole->x2 && // Rectangle's Right Vertical Edge is right to the hole AND

        Rectangle->y2 > Hole->y1 && // Rectangle's Bottom Horizontal Edge is below the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 13;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Rectangle->x1;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 13;

        newHole3 = new HOLE();
        newHole3->x1 = Hole->x1;
        newHole3->y1 = Rectangle->y2;
        newHole3->x2 = Hole->x2;
        newHole3->y2 = Hole->y2;
        newHole3->origin = 13;
    }

    // ⬛⬛⬛
    // ⬛⬛⬛
    // ⬛⬜⬛
    // [CASE 14]: Bottom Rectangle Pop-Out
    if (
        !caseMet &&

        Rectangle->x1 > Hole->x1 && // Rectangle's Left Vertical Edge passes through the hole AND
        Rectangle->x1 < Hole->x2 && //

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && // 

        Rectangle->x2 > Hole->x1 && // Rectangle's Right Vertical Edge passes through the hole AND
        Rectangle->x2 < Hole->x2 && //

        Rectangle->y2 >= Hole->y2 // Rectangle's Bottom Horizontal Edge is below the hole
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Rectangle->x1;
        newHole1->y2 = Hole->y2;
        newHole1->origin = 14;

        newHole2 = new HOLE();
        newHole2->x1 = Hole->x1;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Rectangle->y1;
        newHole2->origin = 14;

        newHole3 = new HOLE();
        newHole3->x1 = Rectangle->x2;
        newHole3->y1 = Hole->y1;
        newHole3->x2 = Hole->x2;
        newHole3->y2 = Hole->y2;
        newHole3->origin = 14;
    }

    // ⬛⬛⬛
    // ⬜⬛⬛
    // ⬛⬛⬛
    // [CASE 15]: Left Rectangle Pop-Out
    if (
        !caseMet &&

        Rectangle->x1 <= Hole->x1 && // Rectangle's Left Vertical Edge is left to the hole AND

        Rectangle->y1 > Hole->y1 && // Rectangle's Top Horizontal Edge passes through the hole AND
        Rectangle->y1 < Hole->y2 && //

        Rectangle->x2 > Hole->x1 && // Rectangle's Bottom Horizontal Edge passes through the hole AND
        Rectangle->x2 < Hole->x2 && //

        Rectangle->y2 > Hole->y1 && // Rectangle's Right Vertical Edge passes through the hole
        Rectangle->y2 < Hole->y2    //
    ) {
        caseMet = true;

        newHole1 = new HOLE();
        newHole1->x1 = Hole->x1;
        newHole1->y1 = Hole->y1;
        newHole1->x2 = Hole->x2;
        newHole1->y2 = Rectangle->y1;
        newHole1->origin = 15;

        newHole2 = new HOLE();
        newHole2->x1 = Rectangle->x2;
        newHole2->y1 = Hole->y1;
        newHole2->x2 = Hole->x2;
        newHole2->y2 = Hole->y2;
        newHole2->origin = 15;

        newHole3 = new HOLE();
        newHole3->x1 = Hole->x1;
        newHole3->y1 = Rectangle->y2;
        newHole3->x2 = Hole->x2;
        newHole3->y2 = Hole->y2;
        newHole3->origin = 15;
    }

    // [SPECIAL CASE] Hole is covered by rectangle
    if (!caseMet) caseMet = true;

    // Delete previous completed Hole //
    // if (caseMet)
    // {
        if (Hole->isCovered(Holes))
        {
            if (newHole1 && !(newHole1->isCovered(Holes))) { 
                newHole1->id = Manager.newHoleID();
                newHole1->appendTo(Holes);
            } else { delete newHole1; }
            if (newHole2 && !(newHole2->isCovered(Holes))) {
                newHole2->id = Manager.newHoleID();
                newHole2->appendTo(Holes);
            } else { delete newHole2; }
            if (newHole3 && !(newHole3->isCovered(Holes))) {
                newHole3->id = Manager.newHoleID();
                newHole3->appendTo(Holes);
            } else { delete newHole3; }
        } else {
            if (newHole1) delete newHole1;
            if (newHole2) delete newHole2;
            if (newHole3) delete newHole3;
        }

        return true;
    // }
    // return false; // We return false (SHOULD never happen) signifying we failed to place Rectangle in Hole
}

// ------------------------------------------------------------------------------------
// EW DRAW
// ------------------------------------------------------------------------------------

// drawOutlined : Draw the border of a rectangle
void EZ::drawOutlined(SHAPE* Rectangle)
{
    SHAPE* ScaledRectangle = new SHAPE();
    ScaledRectangle->id = Rectangle->id*this->RECT_SCALE;
    ScaledRectangle->x1 = Rectangle->x1*this->RECT_SCALE;
    ScaledRectangle->y1 = Rectangle->y1*this->RECT_SCALE;
    ScaledRectangle->x2 = Rectangle->x2*this->RECT_SCALE;
    ScaledRectangle->y2 = Rectangle->y2*this->RECT_SCALE;

    ez_set_color(ez_black);
    ez_set_thick (1);
    ez_draw_rectangle(
        this->WINDOW, 

        ScaledRectangle->x1 + this->HORIZONTAL, 
        ScaledRectangle->y1 + this->VERTICAL, 
        ScaledRectangle->x2 + this->HORIZONTAL, 
        ScaledRectangle->y2 + this->VERTICAL
    );
}
// drawFilled : Draw the whole rectangle (+ border)
void EZ::drawFilled(SHAPE* Rectangle)
{
    if (!Rectangle->Color)
    {
        Rectangle->Color = ez_get_HSV(rand()%360, this->SATURATION, this->VALUE);
    }

    SHAPE* ScaledRectangle = new SHAPE();
    ScaledRectangle->id = Rectangle->id*this->RECT_SCALE;
    ScaledRectangle->x1 = Rectangle->x1*this->RECT_SCALE;
    ScaledRectangle->y1 = Rectangle->y1*this->RECT_SCALE;
    ScaledRectangle->x2 = Rectangle->x2*this->RECT_SCALE;
    ScaledRectangle->y2 = Rectangle->y2*this->RECT_SCALE;

    if (this->RAINBOW) ez_set_color(Rectangle->Color);
    else ez_set_color(ez_green);
    
    ez_fill_rectangle(
        this->WINDOW, 

        ScaledRectangle->x1 + this->HORIZONTAL, 
        ScaledRectangle->y1 + this->VERTICAL, 
        ScaledRectangle->x2 + this->HORIZONTAL, 
        ScaledRectangle->y2 + this->VERTICAL
    );
    drawOutlined(Rectangle); // Black Border

    if (this->SHOW_IDS) 
    {
        ez_set_color (ez_black);
        
        // Convert ID
        string idString = to_string(Rectangle->id);

        int MiddleX = ScaledRectangle->x1 + (ScaledRectangle->x2-ScaledRectangle->x1)/2 - strlen(toCharStar(idString))*2 + this->HORIZONTAL;
        int MiddleY = ScaledRectangle->y1 + (ScaledRectangle->y2-ScaledRectangle->y1)/2 - strlen(toCharStar(idString))*2 + this->VERTICAL;

        ez_draw_text (this->WINDOW, EZ_MC, MiddleX, MiddleY, toCharStar(idString), idString.length());
    }
}
// drawCanvas : Draw/Redraw Canvas with all Rectangles ( Update settings )
void EZ::drawCanvas(RESULT* EndResult)
{
    ez_window_clear(this->WINDOW); // Clear (WHITE) Current Window

    // // Draw All Rectangles
    SHAPES::iterator it;
    for (it = (EndResult->Rectangles).begin(); it != (EndResult->Rectangles).end(); it++)
    {
        drawFilled(it->second);
    }

    // Draw Canvas Rectangle
    if (!CANVAS) {
        CANVAS = new SHAPE();
        CANVAS->x2 = EndResult->Width;
        CANVAS->y2 = EndResult->Height;
    }

    ez_set_color(ez_black);

    string TL = "(0, 0)"; // Top-Left
    ez_draw_text(this->WINDOW, EZ_MC, this->HORIZONTAL - TL.length()*2, this->VERTICAL - 16, toCharStar(TL), TL.length());

    string BL = "(0, " + to_string(EndResult->Height) + ")"; // Bottom-Left
    ez_draw_text(this->WINDOW, EZ_MC, this->HORIZONTAL - BL.length()*2, CANVAS->y2*this->RECT_SCALE + this->VERTICAL + 6, toCharStar(BL), BL.length());

    string TR = "(" + to_string(EndResult->Width) + ", 0)"; // Top-Right
    ez_draw_text(this->WINDOW, EZ_MC, CANVAS->x2*this->RECT_SCALE + this->HORIZONTAL - TR.length()*2, this->VERTICAL - 16, toCharStar(TR), TR.length());

    string BR = "(" + to_string(EndResult->Width) + ", " + to_string(EndResult->Height) + ")"; // Bottom-Right
    ez_draw_text(this->WINDOW, EZ_MC, CANVAS->x2*this->RECT_SCALE + this->HORIZONTAL - BR.length()*2, CANVAS->y2*this->RECT_SCALE + this->VERTICAL + 6, toCharStar(BR), BR.length());

    drawOutlined(CANVAS);
}
// ConsoleControls : Recursive getch() function for (Controlling the Canvas / Updating Settings)
void ConsoleControls(EZ* Instance, RESULT* EndResult)
{
    int key = getch();

    switch (key) {
        case 113: exit(0); break; // 'q' key : Quit
        case 105: Instance->SHOW_IDS = !(Instance->SHOW_IDS); Instance->drawCanvas(EndResult); break; // 'i' key : Show/Hide Rectangle ID's
        case 98: Instance->RAINBOW = !(Instance->RAINBOW); Instance->drawCanvas(EndResult); break; // 'b' key : Toggle Rainbow Rectangles
        case 114: { // 'r' key : Reset Position
            Instance->VERTICAL = Instance->MARGIN;
            Instance->HORIZONTAL = Instance->MARGIN;
            Instance->RECT_SCALE = Instance->DEFAULT_SCALE;

            Instance->drawCanvas(EndResult); 
            break;
        }

        case 43: // '+' key : Zoom In
        {
            Instance->RECT_SCALE += Instance->ZOOM_INC;
            Instance->RECT_SCALE = (Instance->RECT_SCALE < 0) ? 0 : Instance->RECT_SCALE; // Refuse negative scale factor.
            Instance->drawCanvas(EndResult);
            break;
        }
        case 45: // '-' key : Zoom Out
        {
            Instance->RECT_SCALE -= Instance->ZOOM_INC;
            Instance->RECT_SCALE = (Instance->RECT_SCALE < 0) ? 0 : Instance->RECT_SCALE; // Refuse negative scale factor.
            Instance->drawCanvas(EndResult);
            break;
        }

        // PANNING
        case 119: // 'w' key : Move View Up
        {
            Instance->VERTICAL += Instance->PAN_SPEED;
            Instance->drawCanvas(EndResult);
            break;
        }
        case 115: // 's' key : Move View Down
        {
            Instance->VERTICAL -= Instance->PAN_SPEED;
            Instance->drawCanvas(EndResult);
            break;
        }
        case 97: // 'a' key : Move View Left
        {
            Instance->HORIZONTAL += Instance->PAN_SPEED;
            Instance->drawCanvas(EndResult);
            break;
        }
        case 100: // 'd' key : Move View Right
        {
            Instance->HORIZONTAL -= Instance->PAN_SPEED;
            Instance->drawCanvas(EndResult);
            break;
        }
    }

    ConsoleControls(Instance, EndResult);
}

// Main solving function //
void GLOBAL::Solve(int Sort_Strategy)
{
    this->clearShapes();
    this->resetHoleCounter();
    this->resetRectCounter();

    // Define START Hole //
    HOLE* StartHole = new HOLE();

    StartHole->x2 = this->Width;
    StartHole->y2 = INT_INFINITY;
    
    this->newHole(StartHole);
    // ----------------- //

    this->Sort_Strategy = Sort_Strategy;

    switch (Sort_Strategy)
    {
        case 1:
            sort((this->Stored_Rectangles).begin(), (this->Stored_Rectangles).end(), byArea); break;
        case 2:
            sort((this->Stored_Rectangles).begin(), (this->Stored_Rectangles).end(), byHeight); break;
    }
    for (vector<SHAPE*>::iterator it = (this->Stored_Rectangles).begin(); it != (this->Stored_Rectangles).end(); it++)
    {
        SHAPE* Rectangle = *it;
        Rectangle->x2 = (Rectangle->x2) - (Rectangle->x1);
        Rectangle->y2 = (Rectangle->y2) - (Rectangle->y1);

        Rectangle->x1 = 0;
        Rectangle->y1 = 0;

        if (Rectangle->isRotated) Rectangle->Rotate();

        Rectangle->id = this->newRectID();
        Rectangle->appendTo(&(this->Rectangles));
    }

    cout << (this->Rectangles).size() << endl;
    cout << (this->Holes).size() << endl;

    this->Start_Time = high_resolution_clock::now(); // Start Time for Perf.

    // Start Main Iteration over Rectangles //
    SHAPES::iterator R_it;
    for (R_it = this->Rectangles.begin(); R_it != this->Rectangles.end(); R_it++)
    {
        int R_key = R_it->first;
        SHAPE* Rectangle = R_it->second;

        // assert(R_key == Rectangle->id); // The key is normally the Rectangle's id

        SHAPE* bestHole = getBestHole(Rectangle, 1);
        if (bestHole) // ELSE This SHOULD never occur ...
        {
            // Place Current Rectangle in Best Hole //
            int w = (Rectangle->x2 - Rectangle->x1);
            int h = (Rectangle->y2 - Rectangle->y1);

            Rectangle->x2 = bestHole->x1 + w;
            Rectangle->y2 = bestHole->y1 + h;

            Rectangle->x1 = bestHole->x1;
            Rectangle->y1 = bestHole->y1;

            Rectangle->origin = bestHole->origin; // Debugging purposes to test cases that are most redundant, or faulty
            // ----------------------------------- //

            // Cut Holes
            // H_ => Hole
            SHAPES newHoles;
            SHAPES::iterator H_it;

            for (H_it = this->Holes.begin(); H_it != this->Holes.end(); H_it++)
            {
                int H_key = H_it->first;
                SHAPE* Hole = H_it->second;

                // assert(H_key == Hole->id); // The key is normally the hole's id

                if (Rectangle->Overlaps(Hole)) // If the Current Rectangle overlaps with a hole, we break the hole into new holes ...
                {
                    if (createNewHoles(Rectangle, Hole, &newHoles)) continue;

                    printError("Couldn't cut hole " + to_string(Hole->id), 3, NULL);

                    SHAPES* failedShapes = new SHAPES;
                    
                    Rectangle->appendTo(failedShapes);
                    Hole->appendTo(failedShapes);

                    printError("Failed Shapes:", 4, failedShapes);

                    exit(0);
                }
                else Hole->appendTo(&newHoles);
            } 
            this->Holes = newHoles;
        } else {
            printError("Couldn't find hole for rectangle " + to_string(Rectangle->id), 3, NULL);
            exit(0);
        }
    }
    // ------------------------------------ //

    this->End_Time = high_resolution_clock::now(); // End Time for Perf.
    this->Total_Time = this->End_Time - this->Start_Time; // Total Time for Perf.
}
// --------------------- //

// ------------------------------------------------------------------------------------
// MAIN
// ------------------------------------------------------------------------------------

RESULT* EndResult;
void On_Event(Ez_event *ev)
{
    switch (ev->type) {
        case Expose: Draw.drawCanvas(EndResult); break;
        case KeyPress: break;
    }
};
int main(int argc, char* argv[])
{
    // Sanity Checks //
    if (!init_G_(argc, argv)) return 0; // Failed Integrity Check of Globals

    clearConsole();
    resetColor();

    printRunInf();
    // ------------- //



    cout << "Started." << endl;

    // Solving //
    if (Manager.Sort_Strategy == 3)
    {
        vector<RESULT*> Results;

        Manager.Solve(1);                       // Sort Strategy 1
        Results.push_back(Manager.getResult()); //

        Manager.Solve(2);                       // Sort Strategy 2
        Results.push_back(Manager.getResult()); //

        int Smallest_Height = INT_INFINITY;
        for (vector<RESULT*>::iterator it = Results.begin(); it != Results.end(); it++)
        {
            RESULT* CurrentResult = *it;
            if (CurrentResult->Height < Smallest_Height)
            {
                Smallest_Height = CurrentResult->Height;
                EndResult = CurrentResult;
            }
        } 
    } else {
        Manager.Solve(Manager.Sort_Strategy);
        EndResult = Manager.getResult();
    }
    // ------------------------------------ //

    // setColor(10);
    // cout << "\nFinished:\n------------------------------------------------------------\n";
    // resetColor();
    // printRectangles();
    // setColor(10);
    // cout << "------------------------------------------------------------\n";

    setColor(10);
    cout << "\n------------------------------------------------------------\n";
    resetColor();
    printEndInf(EndResult);
    setColor(10);
    cout << "\n------------------------------------------------------------\n";
    resetColor();

    cout << "\nConsole Controls:\n" << endl;
    cout << "--> 'q' to quit" << endl;
    cout << "--> 'i' to Toggle Rectangle ID's\n" << endl;
    cout << "--> 'r' to Reset Position" << endl;
    cout << "--> '+' or '-' to Zoom In/Out" << endl;
    cout << "--> 'w', 'a', 's', 'd' keys respectively to move UP/LEFT/DOWN/RIGHT\n" << endl;
    cout << "--> 'b' to Toggle Rainbow Rectangles" << endl;
    thread t (ConsoleControls, &Draw, EndResult);

    // EZ-DRAW
    if (ez_init() < 0) exit(0);

    Draw.WINDOW = ez_window_create(1280, 720, (Manager.Rotate) ? (LPCSTR)"Solved with rotations!" : (LPCSTR)"Solved without rotations!",  On_Event);

    ez_main_loop();
    exit(0);
}
