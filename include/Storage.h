#ifndef STORAGE_H
#define STORAGE_H

template <class T>
class Storage
{
private:
    static const int CAPACITY = 100;
    T data[CAPACITY];
    int count;
public:
    Storage() : count(0) {}

    Storage(const Storage& other) : count(other.count)
    {
        for (int i = 0; i < count; i++)
            data[i] = other.data[i];
    }

    Storage& operator=(const Storage& other)
    {
        if (this == &other) return *this;
        count = other.count;
        for (int i = 0; i < count; i++)
            data[i] = other.data[i];
        return *this;
    }

    void clear()
    {
        count = 0;
    }

    void add(const T& item)
    {
        if (count >= CAPACITY) return;
        data[count++] = item;
    }

    void removeByID(int id)
    {
        for (int i = 0; i < count; i++)
        {
            if (data[i].getId() == id)
            {
                for (int j = i; j < count - 1; j++)
                    data[j] = data[j + 1];
                count--;
                return;
            }
        }
    }

    T* findByID(int id)
    {
        for (int i = 0; i < count; i++)
            if (data[i].getId() == id)
                return &data[i];
        return nullptr;
    }

    const T* findByID(int id) const
    {
        for (int i = 0; i < count; i++)
            if (data[i].getId() == id)
                return &data[i];
        return nullptr;
    }

    T* getAll() { return data; }

    const T* getAll() const { return data; }

    int size() const { return count; }

    ~Storage() {}
};

#endif
