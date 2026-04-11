#include <backgroundd.h>
#include <film.h>
#include <string>

class API {
private:
    static std::unique_ptr<Background> background;
    static std::unique_ptr<Film> film;

public:
    static void set_background(Background *bg);
    static void set_film(int width, int height);

    static void render();

    static void run();

    static void write_image(const std::string& filename);
};