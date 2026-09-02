#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

/**
 * @file CircularBuffer.h
 * @brief Provides a generic ring buffer implementation.
 */

/**
 * @class CircularBuffer
 * @brief A generic, fixed-capacity circular buffer (ring buffer).
 * 
 * Used internally by storage managers to buffer incoming samples before 
 * performing bulk write operations, minimizing expensive disk I/O.
 * 
 * @tparam T The data type to be stored in the buffer.
 */
template <typename T>
class CircularBuffer {
    size_t m_capacity;     /**< The maximum number of elements the buffer can hold. */
    std::vector<T> m_data; /**< The underlying fixed-size storage array. */
    size_t m_head = 0;     /**< The index where the next element will be written. */
    size_t m_tail = 0;     /**< The index from which the next element will be read. */
    size_t m_size = 0;     /**< The current number of elements in the buffer. */
    bool m_isFull = false; /**< Flag indicating whether the buffer is currently full. */

    /**
     * @brief Validates the capacity during construction.
     * @param cap The requested capacity.
     * @return size_t The capacity if valid.
     * @throws std::invalid_argument If capacity is 0.
     */
    static size_t validateCapacity(size_t cap) {
        if (cap == 0) throw std::invalid_argument("Capacity must be greater than 0");
        return cap;
    }

public:
    /**
     * @brief Constructs a CircularBuffer with a fixed capacity.
     * 
     * @param capacity The maximum number of elements. Must be > 0.
     * @note The 'explicit' keyword prevents implicit type conversions (e.g., from int).
     * Initialization is performed via member initializer lists for performance.
     */
    explicit CircularBuffer(size_t capacity) 
        : m_capacity(validateCapacity(capacity)),
          m_data(capacity) {}

    /**
     * @brief Returns the current number of elements in the buffer.
     * @return size_t The current size.
     */
    size_t size() const {
        return m_size;
    }

    /**
     * @brief Pushes a new item into the buffer.
     * 
     * @param item The item to insert.
     * @return true If the item was successfully inserted.
     * @return false If the buffer is full and the item was rejected.
     */
    bool push(const T& item) {
        if (m_isFull) {
            return false;
        }

        m_data[m_head] = item;
        m_head = (m_head + 1) % m_capacity;
        m_size++;
        if (m_size == m_capacity) {
            m_isFull = true;
        }
        return true;
    }

    /**
     * @brief Pops and returns the oldest item from the buffer.
     * 
     * @return T The oldest item in the buffer.
     * @throws std::runtime_error If the buffer is empty.
     */
    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Buffer is empty");
        }

        T item = m_data[m_tail];
        m_tail = (m_tail + 1) % m_capacity;
        m_size--;
        m_isFull = false;
        return item;
    }

    /**
     * @brief Checks if the buffer is full.
     * @return true If the buffer cannot accept more items.
     */
    bool isFull() const {
        return m_isFull;
    }

    /**
     * @brief Checks if the buffer is empty.
     * @return true If there are no items to pop.
     */
    bool isEmpty() const {
        return m_size == 0;
    }
};