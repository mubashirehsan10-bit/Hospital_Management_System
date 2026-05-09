#ifndef STORAGE_H
#define STORAGE_H

template <class T>
class Storage
{
private:
    T* data;      // pointer instead of array
    int count;
public:
    Storage(int t = 0)
    {
        count = t;
        data = new T[100];  // allocate on heap
    }

    Storage(const Storage& other)
    {
        count = other.count;
        data = new T[100];
        for (int i = 0; i < count; i++)
            data[i] = other.data[i];
    }

    void add(const T& item)
    {
        if (count >= 100) return;
        data[count++] = item;
    }
    void removeByID(int id)
    {
        for (int i = 0; i < count; i++)
        {
            if (data[i].getId() == id) {
                for (int j = i; j < count - 1; j++)
                {
                    data[j] = data[j + 1];
                }
                count--;
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
    T* getAll()
    {
        return data;
    }
    int size()
    {
        return count;
    }

    ~Storage()
    {
        delete[] data;  // free heap memory
    }

    // everything else stays same
};
#endif // !STORAGE_H
