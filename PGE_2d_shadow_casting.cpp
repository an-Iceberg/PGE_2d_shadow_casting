#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <cmath>
#include <vector>
#include <string>

struct line
{
  int x1;
  int y1;
  int x2;
  int y2;
};

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
  std::vector<line> lines;
  std::pair<int, int> selectedIntersection = {-1, -1};

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
      // TODO: creation and deletion of lines in grid positions
      // TODO: modes for creating lines and casting light/shadows
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

  /**
   * @brief All user input is handeled here
   */
  void UserInput()
  {
    // Program can be terminated by pressing escape
    if (GetKey(olc::Key::ESCAPE).bPressed)
    {
      close = true;
    }

    // TODO: check if the intersection isn't occupied by any other line
    // Selects an intersection provided the mouse is in a valid place
    if (GetMouseY() > controlAreaHeight && GetMouse(0).bPressed)
    {
      selectedIntersection.first = FindClosestMultipleOf20(GetMouseX());
      selectedIntersection.second = FindClosestMultipleOf20(GetMouseY());
    }

    // Right click clears the selected intersection
    if (GetMouse(1).bPressed)
    {
      selectedIntersection = {-1, -1};
    }
  }

  /**
   * @brief All internal logic happens here
   */
  void UpdateState()
  {

  }

  /**
   * @brief All drawing routines take place here
   */
  void DrawToScreen()
  {
    Clear(olc::BLACK);

    // Draws the grid
    DrawGrid();

    // Drawing a cricle around the intersection closest to the mouse position
    HighlightNearestIntersection();

    // Draws the selected intersection
    if (selectedIntersection.first != -1 && selectedIntersection.second != -1)
    {
      FillCircle(selectedIntersection.first * gridSize, selectedIntersection.second * gridSize, circleRadius * 0.66f, olc::MAGENTA);
    }

    // Draws the control area with text
    DrawControlArea();
  }

  /**
   * @brief Draws the grid below the control area
   */
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
  /**
   * @brief Highlights the nearest intersection to the mouse position with an orange circle
   */
  void HighlightNearestIntersection()
  {
    if (GetMouseY() > controlAreaHeight)
    {
      // Finding the closest multiples of 20
      int x = FindClosestMultipleOf20(GetMouseX());
      int y = FindClosestMultipleOf20(GetMouseY());

      DrawCircle(x * gridSize, y * gridSize, circleRadius, olc::Pixel(255, 155, 0));
    }
  }

  /**
   * @brief Draws the control area at the top of the screen space
   */
  void DrawControlArea()
  {
    // Drawing the control area at the top of the screen
    FillRect(0, 0, ScreenWidth(), controlAreaHeight, olc::Pixel(115, 0, 125));

    // Draws some text into the control area
    // UI scaling: 2
    DrawString(5, 5, "This is a showcase of 2d shadow casting.", olc::Pixel(0, 255, 255), 2);
  }

  /**
   * @brief Finds the closest multiple of 20 to the input number
   *
   * @param number The position of the mouse
   * @return int The grid coordinate of the number
   */
  int FindClosestMultipleOf20(const int& number)
  {
    int multiplier = number / gridSize;

    // The closest multiple of 20 smaller than number
    int closestSmaller = multiplier * gridSize;

    // The closest multiple of 20 larger than number
    int closestLarger = (multiplier + 1) * gridSize;

    // Returning the one which has a smaller absolute distance to the number
    return (abs(number - closestSmaller) < abs(number - closestLarger)) ? multiplier : multiplier + 1;
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
