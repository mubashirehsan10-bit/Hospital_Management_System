#ifndef STORAGE_H
#define STORAGE_H

template <class T>
class Storage
{
private:
	T data[100];
	int count;
   
public:
    Storage() { count = 0; }
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

};
#endif // !STORAGE_H
