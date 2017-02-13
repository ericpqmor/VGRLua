#ifndef CHRONOS_H
#define CHRONOS_H

class Chronos {
public:
    Chronos(void);
    void reset(void);
    double elapsed(void);
    double time(void);
private:
    double m_reset;
};

#endif // CHRONOS_H
