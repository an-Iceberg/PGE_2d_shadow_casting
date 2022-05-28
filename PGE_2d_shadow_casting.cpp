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

enum DIRECTION
{
  TOP_RIGHT,
  TOP_LEFT,
  BOTTOM_RIGHT,
  BOTTOM_LEFT
};

enum STATE
{
  SELECT_AN_INTERSECTION,
  INTERSECTION_HAS_BEEN_SELECTED,
  CAST_LIGHT
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
  std::vector<line> lines;
  std::pair<int, int> selectedIntersection = {-1, -1};
  int UIscaling = 2;
  STATE state = SELECT_AN_INTERSECTION;

  // Pythagoras' theorem (which wasn't actually invented by Pythagoras himself)
  int diagonalDistance = sqrt(pow((820 - controlAreaHeight), 2) + pow(1280, 2));

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
      // TODO: integrate font extension of the PGE
      // TODO: create some color-constants so that color changes can be applied more easily
      // TODO: fix performance issues for -O0
      // TODO: modes for creating lines and casting light/shadows
      // TODO: store lines using a hash function for efficient lookup times
      UserInput();
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
    // TODO: refactor this mess
    // Drawing mode
    if (state != CAST_LIGHT)
    {
      if (GetKey(olc::RIGHT).bPressed)
      {
        state = CAST_LIGHT;
        return;
      }

      // TODO: refactor with program states in mind
      // Selects an intersection if the mouse is in a valid place
      if (GetMouseY() > controlAreaHeight && GetMouse(0).bPressed)
      {
        // Highlighting a selected intersection
        if (state == SELECT_AN_INTERSECTION)
        {
          selectedIntersection.first = FindClosestMult(GetMouseX());
          selectedIntersection.second = FindClosestMult(GetMouseY());
          state = INTERSECTION_HAS_BEEN_SELECTED;
        }
        // Two intersections have been selected => create a line
        else if (state == INTERSECTION_HAS_BEEN_SELECTED)
        {
          const int selectedX = FindClosestMult(GetMouseX());
          const int selectedY = FindClosestMult(GetMouseY());

          // If the resulting line already exists, do nothing
          for(const auto& line : lines)
          {
            if (
              selectedIntersection.first == line.x1 && selectedIntersection.second == line.y1 && selectedX == line.x2 && selectedY == line.y2 ||
              selectedIntersection.first == line.x2 && selectedIntersection.second == line.y2 && selectedX == line.x1 && selectedY == line.y1
            )
            {
              return;
            }
          }

          lines.push_back({selectedIntersection.first, selectedIntersection.second, selectedX, selectedY});

          // After creating a new line, deselect everything
          selectedIntersection = {-1, -1};
          state = SELECT_AN_INTERSECTION;
        }
      }

      // Right click cancelles a selection or deletes line(s)
      if (GetMouse(1).bPressed)
      {
        // Clear the selection if it is present
        if (state == INTERSECTION_HAS_BEEN_SELECTED)
        {
          selectedIntersection = {-1, -1};
          state = SELECT_AN_INTERSECTION;
        }
        // Delete any lines if either end falls onto the highlighted intersection
        else
        {
          if (!lines.empty())
          {
            const int selectedX = FindClosestMult(GetMouseX());
            const int selectedY = FindClosestMult(GetMouseY());

            for(std::vector<line>::iterator iterator = lines.begin(); iterator != lines.end(); iterator++)
            {
              if (selectedX == iterator->x1 && selectedY == iterator->y1 || selectedX == iterator->x2 && selectedY == iterator->y2)
              {
                lines.erase(iterator);
                iterator--;
              }
            }
          }
        }
      }

      // Pressing backspace clears all lines off the screen
      if (GetKey(olc::BACK).bPressed)
      {
        lines.clear();
      }
    }
    // Light casting mode
    else
    {
      if (GetKey(olc::LEFT).bPressed)
      {
        state = SELECT_AN_INTERSECTION;
        return;
      }
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

    // Draws the grid only when in drawing mode
    if (state != CAST_LIGHT)
    {
      DrawGrid();
    }

    // Draws all created lines
    DrawLines();

    // Drawing a cricle around the intersection closest to the mouse position when in drawing mode
    if (state != CAST_LIGHT)
    {
      HighlightNearestIntersection();
    }
    else
    {
      CastLight();
    }

    // If an intersection has been selected
    if (state == INTERSECTION_HAS_BEEN_SELECTED)
    {
      // Draws a line from the selected intersection to the highlighted one nearest to the mouse
      DrawLine(
        gridSize * FindClosestMult(GetMouseX()),
        gridSize * FindClosestMult(GetMouseY()),
        selectedIntersection.first * gridSize,
        selectedIntersection.second * gridSize,
        olc::CYAN
      );

      // Highlights the selected intersection
      FillCircle(
        selectedIntersection.first * gridSize,
        selectedIntersection.second * gridSize,
        circleRadius * 0.66f,
        olc::MAGENTA
      );
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
    // Drawing vertical lines
    for (int x = gridSize; x < ScreenWidth(); x += gridSize)
    {
      DrawLine(x, controlAreaHeight, x, ScreenHeight(), olc::Pixel(55, 55, 55));
    }

    // Drawing horizontal lines
    for (int y = gridSize + controlAreaHeight; y < ScreenWidth(); y += gridSize)
    {
      DrawLine(0, y, ScreenWidth(), y, olc::Pixel(55, 55, 55));
    }
  }

  /**
   * @brief Draws all the lines stored in lines
   */
  void DrawLines()
  {
    if (state != CAST_LIGHT)
    {
      for(const auto& line : lines)
      {
        DrawLine(line.x1 * gridSize, line.y1 * gridSize, line.x2 * gridSize, line.y2 * gridSize);
      }
    }
    else
    {
      for(const auto& line : lines)
      {
        DrawLine(line.x1 * gridSize, line.y1 * gridSize, line.x2 * gridSize, line.y2 * gridSize, olc::MAGENTA);
      }
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
      // Finding the closest multiples
      int x = FindClosestMult(GetMouseX());
      int y = FindClosestMult(GetMouseY());

      DrawCircle(x * gridSize, y * gridSize, circleRadius, olc::Pixel(255, 155, 0));
    }
  }

  /**
   * @brief Draws the control area at the top of the screen space
   */
  void DrawControlArea()
  {
    // Drawing the control area at the top of the screen
    FillRect(0, 0, ScreenWidth(), controlAreaHeight, olc::Pixel(128, 0, 255));

    // Instructions based on state
    if (state == SELECT_AN_INTERSECTION)
    {
      FillRect(0, 0, 419, 25, olc::DARK_MAGENTA);
      DrawString(5, 5, "Mode: [D]  C  (draw lines)", olc::CYAN, UIscaling);
      DrawString(420, 5, "(change with RIGHT/LEFT)", olc::WHITE, UIscaling);

      DrawString(5, 30, "M1 - select a point", olc::WHITE, UIscaling);
      DrawString(5, 55, "M2 - delete all lines that end at the mouse cursor", olc::WHITE, UIscaling);
      DrawString(5, 80, "BACKSPACE - clear all lines", olc::WHITE, UIscaling);
    }
    else if (state == INTERSECTION_HAS_BEEN_SELECTED)
    {
      FillRect(0, 0, 419, 25, olc::DARK_MAGENTA);
      DrawString(5, 5, "Mode: [D]  C  (draw lines)", olc::CYAN, UIscaling);
      DrawString(420, 5, "(change with RIGHT/LEFT)", olc::WHITE, UIscaling);

      DrawString(5, 30, "M1 - place a line", olc::WHITE, UIscaling);
      DrawString(5, 55, "M2 - cancel", olc::WHITE, UIscaling);
    }
    else if (state == CAST_LIGHT)
    {
      FillRect(0, 0, 419, 25, olc::DARK_MAGENTA);
      DrawString(5, 5, "Mode:  D  [C] (cast light)", olc::CYAN, UIscaling);
      DrawString(420, 5, "(change with RIGHT/LEFT)", olc::WHITE, UIscaling);
    }
  }

  /**
   * @brief Finds the closest multiple to the input number
   *
   * @param number The position of the mouse
   * @return int The grid coordinate of the number
   */
  int FindClosestMult(const int& number)
  {
    int multiplier = number / gridSize;

    // The closest multiple smaller than number
    int closestSmaller = multiplier * gridSize;

    // The closest multiple larger than number
    int closestLarger = (multiplier + 1) * gridSize;

    // Returning the one which has a smaller absolute distance to the number
    return (abs(number - closestSmaller) < abs(number - closestLarger)) ? multiplier : multiplier + 1;
  }

  /**
   * @brief Draws the light
   */
  void CastLight()
  {
    // IDEA: first cast light all over the world, then fill in the shadows behind the structures
    // Fill the area with light
    // ! Don't delete me FillRect(0, controlAreaHeight, ScreenWidth(), ScreenHeight());

    // Then calculate all the shadows
    for (std::vector<line>::iterator iterator = lines.begin(); iterator != lines.end(); iterator++)
    {
      olc::vi2d mousePosition = {GetMouseX(), GetMouseY()};

      olc::vi2d point1 = {iterator->x1 * gridSize, iterator->y1 * gridSize};
      olc::vi2d point2 = {iterator->x2 * gridSize, iterator->y2 * gridSize};

      // HACK: Draw the lines to be just slightly bigger than the screen diagonal, that way the lines can be extended further and no additional math needs to be done
      // Determine the two points of intersection on the edges of the field
      olc::vi2d firstEdgePoint = FindSuitablePoint(mousePosition, point1);
      olc::vi2d secondEdgePoint = FindSuitablePoint(mousePosition, point2);

      std::cout << '\n';

      DrawLine(mousePosition, point1, olc::CYAN);
      DrawLine(mousePosition, point2, olc::CYAN);

      // Divide the resulting polygon into triangles and 🎵 paint it black 🎶
    }
  }

  /**
   * @brief Finds a suitable point to use for completion of the polygon that represents the shadows
   *
   * @param mouseX X pixel position of mouse
   * @param mouseY Y pixel position of mouse
   * @param x X pixel position of a point
   * @param y Y pixel psition of a point
   * @return olc::vi2d The point to be used for the polygon
   */
  olc::vi2d FindSuitablePoint(const olc::vi2d& mouse, const olc::vi2d& point)
  {
    // Calculating the directional vector using AB = B - A
    olc::vi2d direction = point - mouse;

    // FIXME: the maths is very off for the lines
    // Determine the coordinates of the distant point (using u = L / ||v|| * v)
    olc::vi2d solution = (direction * (diagonalDistance / direction.mag())) - mouse;

    std::cout << solution.mag() << ' ';

    return solution;
  }

  void TrianguliseAndPaintItBlack()
  {

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
