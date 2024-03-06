#pragma once

namespace Tmpl8
{
	class Median
	{
	public:
		static int get_median_health(const std::vector<const Tank*>& input, int begin, int end);

	private:
		static int get_median(const int first_value, const int middle_value, const int last_value);
	};
}