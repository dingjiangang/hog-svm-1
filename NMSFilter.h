#pragma once

#include <vector>
#include <tuple>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

namespace nms
{
	typedef tuple<float,float,float,float> CornerCoords;

	void apply(const std::vector<cv::Rect>& srcRects, std::vector<cv::Rect>& resRects, float thresh);
	CornerCoords findMeanCoords(std::vector<CornerCoords> coords);
};

nms::CornerCoords nms::findMeanCoords(std::vector<nms::CornerCoords> coords) {
	CornerCoords ret = std::make_tuple<float,float,float,float>(0,0,0,0);
	for(CornerCoords item : coords){
		std::get<0>(ret) += std::get<0>(item);
		std::get<1>(ret) += std::get<1>(item);
		std::get<2>(ret) += std::get<2>(item);
		std::get<3>(ret) += std::get<3>(item);
	}

	std::get<0>(ret) /= (float)coords.size();
	std::get<1>(ret) /= (float)coords.size();
	std::get<2>(ret) /= (float)coords.size();
	std::get<3>(ret) /= (float)coords.size();

	return ret;
}

void nms::apply(const std::vector<cv::Rect>& srcRects, std::vector<cv::Rect>& resRects, float thresh) {
	resRects.clear();

	const size_t size = srcRects.size();
	if (!size)
	{
		return;
	}

	// Sort the bounding boxes by the bottom - right y - coordinate of the bounding box
	std::multimap<int, size_t> idxs;
	for (size_t i = 0; i < size; ++i)
	{
		idxs.insert(std::pair<int, size_t>(srcRects[i].br().y, i));
	}

	// keep looping while some indexes still remain in the indexes list
	while (idxs.size() > 0)
	{
		// grab the last rectangle
		auto lastElem = --std::end(idxs);
		const cv::Rect& rect1 = srcRects[lastElem->second];

		idxs.erase(lastElem);
		std::vector<CornerCoords> coords;

		for (auto pos = std::begin(idxs); pos != std::end(idxs); )
		{
			// grab the current rectangle
			const cv::Rect& rect2 = srcRects[pos->second];

			float intArea = (rect1 & rect2).area();
			float unionArea = rect1.area() + rect2.area() - intArea;
			float overlap = intArea / unionArea;

			// if there is sufficient overlap, suppress the current bounding box
			if (overlap > thresh)
			{
				coords.push_back(std::make_tuple(rect2.x, rect2.y, rect2.width, rect2.height));
				pos = idxs.erase(pos);
			}
			else
			{
				++pos;
			}
		}

		coords.push_back(std::make_tuple(rect1.x, rect1.y, rect1.width, rect1.height));
		
		//retrieve the new rect coords
		CornerCoords newCoords = findMeanCoords(coords);

		int x = (int)std::get<0>(newCoords);
		int y = (int)std::get<1>(newCoords);
		int width = (int)std::get<2>(newCoords);
		int height = (int)std::get<3>(newCoords);

		//create the rectangle
		Rect rect = Rect(x,y,width,height);

		//add it to the list of hits
		resRects.push_back(rect);
	}
}