#include "precomp.h"

float Median_tank::find_median(std::vector<float>& nums) {

    // Sorteer de vector
    std::sort(nums.begin(), nums.end());

    // Bepaal de index van het middelste element
    int n = nums.size();
    int middle_index = n / 2;

    // Als de grootte van de vector oneven is, is de mediaan het middelste element
    // Als de grootte van de vector even is, is de mediaan het gemiddelde van de twee middelste elementen
    if (n % 2 == 1) {
        return nums[middle_index];
    }
    else {
        return (nums[middle_index - 1] + nums[middle_index]) / 2;
    }
}