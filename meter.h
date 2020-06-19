#if !defined(Fl_Meter_h)
#define Fl_Meter_h

#include <FL/Fl_Widget.H>
#include <vector>

class Fl_Meter : public Fl_Widget {
    public:
        Fl_Meter(int l, int t, int w, int h, char const *lab);
        virtual ~Fl_Meter();

        void copy_data(float const *d, size_t count);
        void calc_range();
        void range(float min, float max);
        void linecolor(Fl_Color c);
        void draw() override;

        std::vector<float> data_;
        Fl_Color lineColor_;
        float minRange_;
        float maxRange_;
};

#endif  //  Fl_Meter_h

