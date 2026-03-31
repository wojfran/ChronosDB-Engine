#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

template <typename T>
class CircularBuffer {
    size_t m_capacity;
    std::vector<T> m_data;
    size_t m_head = 0;
    size_t m_tail = 0;
    bool m_isFull = false;

    static size_t validateCapacity(size_t cap) {
        // notka: nie sprawdzam <= 0 bo size_t jest unsigned, więc nie może być ujemne
        if (cap == 0) throw std::invalid_argument("Capacity must be greater than 0");
        return cap;
    }

    public:
    // notka: explicit zapobieha niejawnej konwersji
    // czyli jeżeli mamy funkcję przyjmującą CircularBuffer, 
    // to jezeli damy jej int, to nie będzie próbowała konwertować 
    // tego inta na CircularBuffer, tylko od razu będzie błąd kompilacji 
    // dodatkowo robimy inicjalizację w liście inicjalizacyjnej, co jest 
    // szybsze (tworzymy obiekt z ustawionymi atrybutami, zamiast tworzyć pusty i potem ustawiać atrybuty) 
    explicit CircularBuffer(size_t capacity) 
        : m_capacity(validateCapacity(capacity)),
          m_data(capacity) {}

    bool push(const T& item) {
        if (m_isFull) {
            return false;
        }

        m_data[m_head] = item;
        m_head = (m_head + 1) % m_capacity;
        if (m_head == m_tail) {
            m_isFull = true;
        }
        return true;
    }

    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Buffer is empty");
        }

        T item = m_data[m_tail];
        m_tail = (m_tail + 1) % m_capacity;
        m_isFull = false;
        return item;
    }

    bool isFull() const {
        return  m_isFull;
    }

    bool isEmpty() const {
        return !m_isFull && (m_head == m_tail);
    }
};