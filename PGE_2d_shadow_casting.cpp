#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <cmath>
#include <vector>
#include <string>

class PGE_2d_shadow_casting : public olc::PixelGameEngine
{
public:
  PGE_2d_shadow_casting()
  {
    sAppName = "2D Shadow Casting";
  }

private:
  int circleRadius = 10;
  int gridSize = 20;
  int controlAreaHeight = 100;
  bool close = false;

public:
  bool OnUserCreate() override
  {
    Clear(olc::BLACK);

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    if (IsFocused())
    {
      UserInput();

      // Closes the program
      if (close)
      {
        return false;
      }

      UpdateState();
      DrawToScreen();
    }

    return true;
  }

  void UserInput()
  {
    // Program can be terminated by pressing escape
    if (GetKey(olc::Key::ESCAPE).bPressed)
    {
      close = true;
    }
  }

  void UpdateState()
  {

  }

  void DrawToScreen()
  {
    Clear(olc::BLACK);

    // Draws the grid
    DrawGrid();

    // Drawing a cricle around the intersection closest to the mouse position
    HighlightNearestIntersection();

    // Drawing the control area at the top of the screen
    FillRect(0, 0, ScreenWidth(), controlAreaHeight, olc::Pixel(115, 70, 120));
  }

  void DrawGrid()
  {
    // Grid field is 1280x720px with 100px at the top to display text
    // Drawing vertical lines every 10 pixels
    for (int x = gridSize; x < ScreenWidth(); x += gridSize)
    {
      DrawLine(x, controlAreaHeight, x, ScreenHeight(), olc::Pixel(55, 55, 55));
    }

    // Drawing horizontal lines every 10 pixels
    for (int y = gridSize + controlAreaHeight; y < ScreenWidth(); y += gridSize)
    {
      DrawLine(0, y, ScreenWidth(), y, olc::Pixel(55, 55, 55));
    }
  }

  // TODO {optional}: implement deadzone
  // Highlights the nearest intersection to the mouse position with an orange circle
  void HighlightNearestIntersection()
  {
    if (GetMouseY() > controlAreaHeight)
    {
      // Finding the closest multiples of 20
      int x = FindClosestMultipleOf20(GetMouseX());
      int y = FindClosestMultipleOf20(GetMouseY());

      DrawCircle(x, y, circleRadius, olc::Pixel(255, 155, 0));
    }
  }

  // Finds the closest multiple of 20 to the input number
  int FindClosestMultipleOf20(const int& number)
  {
    int multiplier = number / gridSize;

    // The closest multiple of 20 smaller than number
    int closestSmaller = multiplier * gridSize;

    // The closest multiple of 20 larger than number
    int closestLarger = (multiplier + 1) * gridSize;

    // Returning the one which has a smaller absolute distance to the number
    return (abs(number - closestSmaller) < abs(number - closestLarger)) ? closestSmaller : closestLarger;
  }
};

int main()
{
  PGE_2d_shadow_casting demo;

  if (demo.Construct(1280, 820, 1, 1, false, true))
  {
    demo.Start();
  }

  return 0;
}
