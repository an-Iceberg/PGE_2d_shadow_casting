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
  const int circleRadius = 10;
  const int gridSize = 20;
  const int controlAreaHeight = 100;
  const int UIscaling = 2;

  std::vector<line> lines;
  olc::vi2d selectedIntersection = {-1, -1};
  olc::vi2d mouse;

  STATE state = SELECT_AN_INTERSECTION;

  int diagonalDistance;
  int screenWidth;
  int screenHeight;

public:
  bool OnUserCreate() override
  {
    Clear(olc::BLACK);

    diagonalDistance = sqrt(pow(ScreenHeight(), 2) + pow(ScreenWidth(), 2));
    screenWidth = ScreenWidth();
    screenHeight = ScreenHeight();

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    if (IsFocused())
    {
      mouse = {GetMouseX(), GetMouseY()};

      // TODO: integrate font extension of the PGE
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
      if (mouse.y > controlAreaHeight && GetMouse(0).bPressed)
      {
        // Highlighting a selected intersection
        if (state == SELECT_AN_INTERSECTION)
        {
          selectedIntersection = {FindClosestMult(mouse.x), FindClosestMult(mouse.y)};
          state = INTERSECTION_HAS_BEEN_SELECTED;
        }
        // Two intersections have been selected => create a line
        else if (state == INTERSECTION_HAS_BEEN_SELECTED)
        {
          const olc::vi2d selected = {FindClosestMult(mouse.x), FindClosestMult(mouse.y)};

          // If the resulting line already exists, do nothing
          for(const auto& line : lines)
          {
            if (
              selectedIntersection.x == line.x1 && selectedIntersection.y == line.y1 && selected.x == line.x2 && selected.y == line.y2 ||
              selectedIntersection.x == line.x2 && selectedIntersection.y == line.y2 && selected.x == line.x1 && selected.y == line.y1
            )
            {
              return;
            }
          }

          lines.push_back({selectedIntersection.x, selectedIntersection.y, selected.x, selected.y});

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
            const olc::vi2d selected = {FindClosestMult(mouse.x), FindClosestMult(mouse.y)};

            for(std::vector<line>::iterator iterator = lines.begin(); iterator != lines.end(); iterator++)
            {
              if (selected.x == iterator->x1 && selected.y == iterator->y1 || selected.x == iterator->x2 && selected.y == iterator->y2)
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

    // Drawing a cricle around the intersection closest to the mouse position when in drawing mode
    if (state != CAST_LIGHT)
    {
      HighlightNearestIntersection();
    }
    else
    {
      CastLight();
    }

    // Draws all created lines
    DrawLines();

    // If an intersection has been selected
    if (state == INTERSECTION_HAS_BEEN_SELECTED)
    {
      // Draws a line from the selected intersection to the highlighted one nearest to the mouse
      DrawLine({gridSize * FindClosestMult(mouse.x), gridSize * FindClosestMult(mouse.y)}, selectedIntersection * gridSize, olc::CYAN);

      // Highlights the selected intersection
      FillCircle(selectedIntersection * gridSize, circleRadius * 0.66f, olc::MAGENTA);
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
    for (int x = gridSize; x < screenWidth; x += gridSize)
    {
      DrawLine(x, controlAreaHeight, x, screenHeight, olc::Pixel(55, 55, 55));
    }

    // Drawing horizontal lines
    for (int y = gridSize + controlAreaHeight; y < screenWidth; y += gridSize)
    {
      DrawLine(0, y, screenWidth, y, olc::Pixel(55, 55, 55));
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
    // else
    // {
    //   for(const auto& line : lines)
    //   {
    //     DrawLine(line.x1 * gridSize, line.y1 * gridSize, line.x2 * gridSize, line.y2 * gridSize, olc::MAGENTA);
    //   }
    // }
  }

  // TODO {optional}: implement deadzone
  /**
   * @brief Highlights the nearest intersection to the mouse position with an orange circle
   */
  void HighlightNearestIntersection()
  {
    if (mouse.y > controlAreaHeight)
    {
      // Finding the closest multiples
      const olc::vi2d point = {FindClosestMult(mouse.x), FindClosestMult(mouse.y)};

      DrawCircle(point.x * gridSize, point.y * gridSize, circleRadius, olc::Pixel(255, 155, 0));
    }
  }

  /**
   * @brief Draws the control area at the top of the screen space
   */
  void DrawControlArea()
  {
    // Drawing the control area at the top of the screen
    FillRect(0, 0, screenWidth, controlAreaHeight, olc::Pixel(128, 0, 255));

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
    const int multiplier = number / gridSize;

    // The closest multiple smaller than number
    const int closestSmaller = multiplier * gridSize;

    // The closest multiple larger than number
    const int closestLarger = (multiplier + 1) * gridSize;

    // Returning the one which has a smaller absolute distance to the number
    return (abs(number - closestSmaller) < abs(number - closestLarger)) ? multiplier : multiplier + 1;
  }

  /**
   * @brief Draws the light
   */
  void CastLight()
  {
    // TODO: implement javidx9's solution
    // Calculates all triangles that should represent the light
    for (std::vector<line>::iterator line = lines.begin(); line != lines.end(); line++)
    {
      const olc::vi2d invalid = {-1, -1};

      const olc::vi2d point1 = {line->x1 * gridSize, line->y1 * gridSize};
      const olc::vi2d point2 = {line->x2 * gridSize, line->y2 * gridSize};

      // Determine two points outside the viewport
      const olc::vi2d distantPoint1 = FindSuitablePoint(point1);
      const olc::vi2d distantPoint2 = FindSuitablePoint(point2);

      int distance1;
      int distance2;

      olc::vi2d distance;

      DrawLine(mouse, distantPoint1, olc::MAGENTA);
      DrawLine(mouse, distantPoint2, olc::MAGENTA);

      DrawLine(point1, point2, olc::CYAN);
    }
  }

  /**
   * @brief Finds a suitable point to use for completion of the polygon that represents the shadows
   *
   * @param olc::vi2d Coordinates of the point
   * @return olc::vi2d The point to be used for the polygon
   */
  olc::vi2d FindSuitablePoint(const olc::vi2d& point)
  {
    // Edgecase
    if (mouse == point)
    {
      return point;
    }

    // Calculating the directional vector using AB = B - A
    // Determine the coordinates of the distant point (using u = L / ||v|| * v)
    return ((float)diagonalDistance / (float)(point - mouse).mag() * (point - mouse)) + mouse;
  }

  /**
   * @brief Finds the intersection with one of the four walls
   *
   * @param point
   * @param distantPoint
   * @return olc::vi2d Coordinates of the intersection
   */
  olc::vi2d FindIntersection(const olc::vi2d& point, const olc::vi2d& distantPoint)
  {
    olc::vi2d badResult = {-1, -1};
    olc::vi2d topLeft = {0, controlAreaHeight};
    olc::vi2d bottomLeft = {0, screenHeight};
    olc::vi2d bottomRight = {screenWidth, screenHeight};
    olc::vi2d topRight = {screenWidth, controlAreaHeight};

    // TODO: account for the occasional 99 retunr value
    // TODO: Determine which wall the line intersects with
    // Intersection with the west wall
    olc::vi2d intersection = CalculateIntersection(point, distantPoint, topLeft, bottomLeft);

    if (intersection != badResult)
    {
      return intersection;
    }

    // Intersection with the north wall
    intersection = CalculateIntersection(point, distantPoint, topLeft, topRight);

    if (intersection != badResult)
    {
      return intersection;
    }

    // Intersection with the east wall
    intersection = CalculateIntersection(point, distantPoint, topRight, bottomRight);

    if (intersection != badResult)
    {
      return intersection;
    }

    // Intersection with the south wall
    intersection = CalculateIntersection(point, distantPoint, bottomLeft, bottomRight);

    return intersection;
  }

  /**
   * @brief Calculates the intersection between two lines represented by two points each
   *
   * @param p0 Point 0
   * @param p1 Point 1
   * @param p2 Point 2
   * @param p3 Point 3
   * @return olc::vi2d The intersection
   */
  olc::vi2d CalculateIntersection(const olc::vf2d& p0, const olc::vf2d& p1, const olc::vf2d& p2, const olc::vf2d& p3)
  {
    const olc::vf2d s1 = p1 - p0;
    const olc::vf2d s2 = p3 - p2;

    const float s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
    const float t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);

    olc::vi2d intersection = {-1, -1};

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
      // Collision detected
      intersection.x = p0.x + (t * s1.x);
      intersection.y = p0.y + (t * s1.y);
    }

    if (intersection == p0 || intersection == p1 || intersection == p2 || intersection == p3)
    {
      return {-1, -1};
    }

    return intersection;
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
