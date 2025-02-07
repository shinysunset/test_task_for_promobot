/*


Требуется реализовать программу, которая упорядочит массив объектов꞉
1. Создается несколько потоков, задача каждого ‑ упорядочить один и тот же
массив (берет единый список ‑ упорядочивает его ‑ выводит в консоль
упорядоченный массив). Разница в потоках ‑ разный алгоритм сортировки, т.е.
вывод в консоль должен быть примерно такой꞉
> Сортировка꞉ быстрая
> Результат꞉ 1,2,3,4
> Сортировка꞉ пузырьком
> Результат꞉ 1,2,3,4
2. Массив должен состоять из объектов "геометрические фигуры" (круг,
треугольник, прямоугольник). Реализовать классы требуется через наследование
от абстрактного класса и реализовать в каждом метод расчет


*/

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <Windows.h>

using namespace std;

// Абстрактный класс геометрической фигуры
class Shape {
public:
    virtual ~Shape() = default;
    virtual double calculateArea() const = 0;
    virtual wstring getName() const = 0;
    virtual void print(wostream& os) const = 0;
};

// Круг
class Circle : public Shape {
public:
    Circle(double radius) : radius(radius) {
        if (radius <= 0) throw invalid_argument("Радиус должен быть положительным");
    }

    double calculateArea() const override {
        return M_PI * radius * radius;
    }

    wstring getName() const override {
        return L"Круг";
    }

    void print(wostream& os) const override {
        os << getName() << L" (радиус: " << radius << L", площадь: " << calculateArea() << L")";
    }

private:
    double radius;
};

// Прямоугольник
class Rectangle : public Shape {
public:
    Rectangle(double width, double height) : width(width), height(height) {
        if (width <= 0 || height <= 0) throw invalid_argument("Стороны должны быть положительными");
    }

    double calculateArea() const override {
        return width * height;
    }

    wstring getName() const override {
        return L"Прямоугольник";
    }

    void print(wostream& os) const override {
        os << getName() << L" (" << width << L"x" << height << L", площадь: " << calculateArea() << L")";
    }

private:
    double width, height;
};

// Треугольник
class Triangle : public Shape {
public:
    Triangle(double a, double b, double c) : a(a), b(b), c(c) {
        if (a <= 0 || b <= 0 || c <= 0)
            throw invalid_argument("Стороны должны быть положительными");
        if (!(a + b > c && a + c > b && b + c > a))
            throw invalid_argument("Некорректные стороны треугольника");
    }

    double calculateArea() const override {
        double p = (a + b + c) / 2;
        return sqrt(p * (p - a) * (p - b) * (p - c));
    }

    wstring getName() const override {
        return L"Треугольник";
    }

    void print(wostream& os) const override {
        os << getName() << L" (" << a << L", " << b << L", " << c << L", площадь: " << calculateArea() << L")";
    }

private:
    double a, b, c;
};

// Базовый класс стратегии сортировки
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(vector<shared_ptr<Shape>>& arr) const = 0;
    virtual wstring name() const = 0;
};

// Быстрая сортировка
class QuickSort : public SortStrategy {
public:
    void sort(vector<shared_ptr<Shape>>& arr) const override {
        if (arr.empty()) return;
        quickSort(arr, 0, static_cast<int>(arr.size()) - 1);
    }

    wstring name() const override { return L"быстрая"; }

private:
    void quickSort(vector<shared_ptr<Shape>>& arr, int low, int high) const {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi - 1);
            quickSort(arr, pi + 1, high);
        }
    }

    int partition(vector<shared_ptr<Shape>>& arr, int low, int high) const {
        double pivot = arr[high]->calculateArea();
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j]->calculateArea() < pivot) {
                swap(arr[++i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[high]);
        return i + 1;
    }
};

// Пузырьковая сортировка
class BubbleSort : public SortStrategy {
public:
    void sort(vector<shared_ptr<Shape>>& arr) const override {
        for (size_t i = 0; i < arr.size(); ++i) {
            for (size_t j = 0; j < arr.size() - i - 1; ++j) {
                if (arr[j]->calculateArea() > arr[j + 1]->calculateArea()) {
                    swap(arr[j], arr[j + 1]);
                }
            }
        }
    }

    wstring name() const override { return L"пузырьком"; }
};

// Генератор случайных фигур
vector<shared_ptr<Shape>> generateShapes(size_t count) {
    vector<shared_ptr<Shape>> shapes;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> typeDist(0, 2);
    uniform_real_distribution<> sizeDist(1.0, 10.0);

    for (size_t i = 0; i < count; ++i) {
        bool shapeCreated = false;
        while (!shapeCreated) {
            try {
                switch (typeDist(gen)) {
                case 0:
                    shapes.push_back(make_shared<Circle>(sizeDist(gen)));
                    break;
                case 1:
                    shapes.push_back(make_shared<Rectangle>(sizeDist(gen), sizeDist(gen)));
                    break;
                case 2: {
                    double a = sizeDist(gen);
                    double b = sizeDist(gen);
                    double c = sizeDist(gen);
                    shapes.push_back(shared_ptr<Shape>(new Triangle(a, b, c)));
                    break;
                }
                }
                shapeCreated = true;
            }
            catch (const invalid_argument& e) {
                // Пропускаем невалидные фигуры
            }
        }
    }
    return shapes;
}


// Потокобезопасный логгер
class ThreadSafeLogger {
public:
    void log(const wstring& message) {
        lock_guard<mutex> lock(mtx_);
        wcout << message << endl;
    }

private:
    mutex mtx_;
};

// Функция для потока
void sortAndLog(const vector<shared_ptr<Shape>>& original,
    const SortStrategy& strategy,
    ThreadSafeLogger& logger) {
    auto arr = original;
    strategy.sort(arr);

    wostringstream oss;
    oss << L"Сортировка: " << strategy.name() << L"\nРезультат:\n";
    for (const auto& shape : arr) {
        shape->print(oss);
        oss << L"\n";
    }
    logger.log(oss.str());
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "ru_RU.UTF-8");

    constexpr size_t SHAPES_COUNT = 5;
    auto shapes = generateShapes(SHAPES_COUNT);
    ThreadSafeLogger logger;

    QuickSort quick;
    BubbleSort bubble;

    vector<thread> threads;
    threads.emplace_back(sortAndLog, cref(shapes), cref(quick), ref(logger));
    threads.emplace_back(sortAndLog, cref(shapes), cref(bubble), ref(logger));

    for (auto& t : threads) t.join();

    return 0;
}