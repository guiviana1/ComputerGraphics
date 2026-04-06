#include <array>
#include <bits/stdc++.h>

class RGBColor
{
public:
    double r, g, b;
};

class BackgroundColor {
  private:
    /// Each corner has a color associated with.
    std::array<RGBColor,4> corners{RGBColor{0,0,0},RGBColor{0,0,0},RGBColor{0,0,0},RGBColor{0,0,0}};
    
    /// Corner indices.
    enum Corners_e {
      bl=0, //!< Bottom left corner. / indice 0
      tl,   //!< Top left corner. / indice 1
      tr,   //!< Top right corner. / indice 2
      br    //!< Bottom right corner. / indice 3
    };

    /// Return the linearly interpolated color in [A;B], based on the parameter \f$0\leq t \leq 1.\f$
    RGBColor lerp( const RGBColor &A, const RGBColor &B, double t ) const;

  public:
    /// Ctro receives a list of four colors, for each corner.
    BackgroundColor( const std::vector< RGBColor >& colors );

    /// Dtro
    ~BackgroundColor();

    /// Sample and returns a color, based on the raster coordinate.
    RGBColor sampleUV( int u, int v, double width, double height ) const;
 
    void show_corners(BackgroundColor bkg){
      for(int j = 0; j < corners.size(); j++) {
        std::cout << corners[j].r << " " << corners[j].g << " " << corners[j].b << " I ";
      }
    }
};