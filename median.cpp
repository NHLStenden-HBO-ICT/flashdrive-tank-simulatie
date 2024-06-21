#include "precomp.h" 

/// <summary>
/// Get median from 3 values
/// </summary>
/// <param name="first_value"></param>
/// <param name="middle_value"></param>
/// <param name="last_value"></param>
/// <returns>Median</returns>
int Median::get_median(const int first_value, const int middle_value, const int last_value)
{

    bool begin_is_median = first_value > middle_value && first_value < last_value
                            || first_value < middle_value && first_value > last_value;

    bool middle_is_median = middle_value > first_value && middle_value < last_value
                            || middle_value < first_value && middle_value > last_value;

    if (begin_is_median)
    {
        return first_value;
    }

    else if (middle_is_median)
    {
        return middle_value;
    }

    return last_value;
}

/// <summary>
/// Get median from a list of tanks
/// </summary>
/// <param name="input">List of tanks</param>
/// <param name="begin">Begin of list</param>
/// <param name="end">End of list</param>
/// <returns></returns>
int Median::get_median_health(const vector<const Tank*>& input, int begin, int end)
{
    boolean list_has_2_elements = end - begin == 2;
    if (list_has_2_elements)
    {
        return (int)input.at(begin)->health;
    }

    const int center = (end + begin) / 2;

    // th = tank health
    const int first_value = (int)input.at(begin)->health;
    const int middle_value = (int)input.at(center)->health;
    const int last_value = (int)input.at(end - 1)->health;

    int median = get_median(first_value, middle_value, last_value);

    return median;
}

