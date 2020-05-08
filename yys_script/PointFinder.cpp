#include "PointFinder.h"

#include <QApplication>
#include <QPixmap>
#include <QScreen>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QImage>
#include <QDebug>
#define RANGE_EXP(a,b) abs((a) - (b)) <= 10
LPARAM PointFinder::TransaltePointToLPARAM(QPoint point)
{
	LPARAM ret = point.y();
	ret = ret << 16;
	ret += point.x();
	return ret;
}

auto PointFinder::get_discovery_pos(HWND hd) -> QPoint
{

	return QPoint(-1,-1);
}

QPoint PointFinder::get_chapter_pos(const HWND hd)
{
	auto image_name = QString::fromLocal8Bit("./assets/17.jpg");
	return find_pos(hd, image_name);
}

QPoint PointFinder::get_explore_pos(HWND hd)
{
	auto image_name = QString::fromLocal8Bit("./assets/explore_button.jpg");
	return find_pos(hd, image_name);
}

bool PointFinder::check_color(QColor& color1, QColor& color2)
{
	const auto r1 = color1.red();
	const auto b1 = color1.blue();
	const auto g1 = color1.green();
	const auto r2 = color2.red();
	const auto b2 = color2.blue();
	const auto g2 = color2.green();
	if (RANGE_EXP(r1,r2) && RANGE_EXP(b1,b2) && RANGE_EXP(g1,g2))
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool PointFinder::check_image(QImage& src, QImage& dst, QPoint& pos)
{
	int same_point = 0;
	auto total_point = dst.width()*dst.height();
	int x1 = pos.x(), y1 = pos.y(), x2 = 0, y2 = 0;
	while (x1 < src.width() && y1 < src.height() && x2 < dst.width() && y2 < dst.height())
	{
		if (check_color(src.pixelColor(x1, y1), dst.pixelColor(x2, y2)))
		{	
			same_point++;
		}
		x1++;
		y1++;
		x2++;
		y2++;
	}
	qDebug() << same_point << "/" << total_point;
	if (static_cast<double>(same_point)/total_point < 0.95)
	{
		return false;
	}
	return true;
}

QPoint PointFinder::find_pos(HWND hd, QString& dst_name)
{
	IplImage *src, *templat, *result, *show;
	int srcW, templatW, srcH, templatH, resultW, resultH;
	//加载源图像
	auto screen = QGuiApplication::primaryScreen();
	const auto map = screen->grabWindow(reinterpret_cast<WId>(hd)/*,578,122,41,39*/);
	map.save("shots.jpg","JPG");
	auto screenshots = map.toImage();

	// 将QImage转换为Image
	src = cvLoadImage("shots.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	
	//用于显示结果
	show = cvLoadImage("shots.jpg");
	// cvNamedWindow("show");
	// cvShowImage("show", show);
	//加载模板图像
	templat = cvLoadImage(dst_name.toStdString().c_str(), CV_LOAD_IMAGE_GRAYSCALE);

	if (!src || !templat)
	{
		printf("打开图片失败");
		return{};
	}

	srcW = src->width;
	srcH = src->height;

	templatW = templat->width;
	templatH = templat->height;

	if (srcW < templatW || srcH < templatH)
	{
		printf("模板不能比原图大");
		return{};
	}

	//计算结果矩阵的大小
	resultW = srcW - templatW + 1;
	resultH = srcH - templatH + 1;
	result = cvCreateImage(cvSize(resultW, resultH), 32, 1);
	double minVal, maxVal;
	CvPoint minLoc, maxLoc;

	cvMatchTemplate(src, templat, result, CV_TM_SQDIFF);
	//查找最相似的值及其所在坐标
	cvMinMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, NULL);
	auto x = minLoc.x + (templat->width) / 2;
	auto y = minLoc.y + (templat->height) / 2;
	const auto ret = QPoint(x, y);
	//绘制结果 ;
	//cvRectangle(show, minLoc, cvPoint(minLoc.x + templat->width, minLoc.y + templat->height), CV_RGB(0, 255, 0), 1);
	//
	////显示结果
	// cvNamedWindow("show");
	// cvNamedWindow("tem");
	// cvShowImage("show", show);
	// cvShowImage("tem", templat);
	return ret;
}

auto PointFinder::qimage_to_iplimage(const QImage* image) -> IplImage*
{
	const int width = image->width();
	const int height = image->height();
	CvSize size;
	size.height = height;
	size.width = width;
	IplImage *IplImageBuffer = cvCreateImage(size, IPL_DEPTH_8U, 3);
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			QRgb rgb = image->pixel(x, y);
			cvSet2D(IplImageBuffer, y, x, 0.299 * qRed(rgb) + 0.587 * qGreen(rgb) + 0.114 * qBlue(rgb));
		}
	}
	return IplImageBuffer;
}
