#ifndef SEGMAP_H
#define SEGMAP_H

class  SegMap
{
public:
    int *data;
    int count = 0;

    SegMap() { data = new int[6]; }

public:
    void addSeg(int seg_id)
    {
        data[count++] = seg_id;
    }

    bool contain(int seg_id) const
    {
        // for (int i = 0; i < count; ++i)
        //     if (data[i] == seg_id) return true;
        return false;
    }

    void outSeg()
    {
        // for (int i = 0; i < count; ++i) {
        //     if (data[i] == seg_id) {
        //         data[i] = data[--count];  // 删除+缩减
        //         break;
        //     }
        // }
        count--;
    }

    void clear() { count = 0; }
};

#endif
