// Napisz definicję klasy pochodnej do podanej, która będzie miała metodę clear (ustawiającą wartość 0 we wszystkich polach FVect) oraz add (przyjmującą jeden
// parametr typu double i dodającą go do wszystkich pól wektora).
#include <string>
#include<iostream>
class A {
    private:
    double FVect[3];

    public:
    virtual void setValue(int _idx, double _val) { FVect[_idx] =_val; }
    virtual double value(int _idx) { return FVect[_idx];}
};

class B : public A {
    public:
    void clear() {
        for(int i = 0 ; i < 3; i++) {
            setValue(i, 0);
        }
    }

    void add(double toAdd) {
        for (int i = 0; i < 3; i++) {
            double newVal = value(i) + toAdd;
            setValue(i, newVal);
        }
    }
};

// Przygotuj szablon metody (nie szablon klasy) dodającej do pola Value wartość typu T (T jest parametrem szablonu) dla podanej niżej klasy:
class A {
    double Value = 0;
    public:
    template <typename T> 
    void addValue(T element){
        Value = Value + <double>element;
    }
};
// tu uzupełnić definicję klasy

// Uzupełnij podaną klasę o kod operatora, by kod poniżej skompilował się prawidłowo i wydrukował dziesięć kropek:
class Generator {
    private:
    int Aktualny;
    public: 
    Generator(int i = 10) : Aktualny(i) { }
    int operator() () {
        Aktualny += 10;
        return Aktualny;
    }
};

int main(int argc, char *argv[])
{
    Generator g(10);
    do {
        std::cout << '.';
    } while (g() < 101);
};

// Uzupełnij podaną klasę o kod operatora, by kod poniżej skompilował się prawidłowo i wydrukował
// co drugą małą literę:
class Generator {
    private: 
    char Aktualny;
    public: 
    Generator(char _a = 'A') : Aktualny(_a) { }
    char operator() () {
        char curr = Aktualny;
        Aktualny += 2;
        return curr;
    }
};

int main(int argc, char *argv[] )
{
    Generator g('a'); // to i następne jest w main:
    char czn = g();
    while (czn <= 'z') {
        std::cout << czn;
        czn=g();
    };

};

// Zmodyfikuj kod klasy z zadania 1 tak, by w przypadku wyjścia poza zakres tablicy był rzucany wyjątek. Podaj sensowny przykład przechwycenia tego wyjątku.
class A {
    private:
    double FVect[3];

    public:
    virtual void setValue(int _idx, double _val) { FVect[_idx] =_val; }
    virtual double value(int _idx) { return FVect[_idx];}
};

// Przygotuj szablon funkcji, liczącej sumę elementów wybieranych co x pochodzących z podanego zakresu pocz,
// kon z dowolnego kontenera STL (x ma być parametrem metody a nie pozatypowym parametrem szablonu).
template <typename T>
auto sumEveryX(const T& container, int x, int pocz, int kon) {
    // if (x < 1) x = 1; // Lub throw std::invalid_argument("x musi byc > 0");
    if (x < 1) {
        throw std::invalid_argument("x musi być > 0")
    }

    using ValueType = typename T::value_type;
    ValueType sum = 0;

    auto it = container.begin();
    int i = 0;

    while (it != container.end() && i < pocz) {
        it++;
        i++;
    }

    while (it != container.end() && i <kon) {
        sum += *it;

        for (int step = 0; it != container.end() && step < x; step++){
            it++;
            i++;
        }
    }

    return sum;
}

// Napisz definicję klasy pochodnej do podanej, która będzie miała metodę clear (ustawiającą wartość wszystkich napisów na napis pusty) oraz rotate (cyklicznie
// przesuwającą wszystkie napisy w wektorze o 1 w dowolną stronę).
class CCos {
    private:
    std::string FVect[3];
    public:
    virtual void setValue(int _idx, std::string _val) { FVect[_idx] =_val; }
    virtual std::string value(int _idx) { return FVect[_idx]; }
};

class CDos : public CCos {
    public:
    void clear() {
        for (int i = 0; i < 3; i++) {
            setValue(i, "pusty");
        }
    }
    void rotate(bool toLeft = false) {
        std::string temp = value(0);
        int next;
        for (int i = 0; i < 3; i++) {
            next = (i+1) % 3;
            std::string nextVal = value(next);
            setValue(next, temp);
            temp = nextVal;
        }   
    }
};

// Napisz definicję klasy pochodnej do podanej, w której będziemy mieli pewność iż pole masa ma zawsze wartość
// większą od zera.
class CCos2 {
    private:
    double masa;
    protected:
    virtual void setMasa(double _i) { masa = _i; }
    public:
    CCos2 () {};
};

class CCosiek : CCos2 {
    public:
    CCosiek(double _masa) {
        setMasa(_masa);
    }
    
    void setMasa(double _i) override {
        if (_i > 0) {
            setMasa(_i);
        } else {
            throw std::invalid_argument("masa musi byc wieksza niz 0");
        }
    }
};