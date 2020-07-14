#ifndef GRSTAPS_UTILS_HPP
#define GRSTAPS_UTILS_HPP

#include <limits>
#include <cstdint>

#define DOMAIN_CONCURRENT	0
#define DOMAIN_DEAD_ENDS	1
#define DOMAIN_REVERSIBLE	2

#define SEARCH_G_HFF		0
#define SEARCH_G_2HFF		1
#define SEARCH_HFF			2
#define SEARCH_G_3HFF		3
#define SEARCH_G_HLAND_HFF	4
#define SEARCH_G_2HAUX		5
#define SEARCH_G_HLAND		10
#define SEARCH_G_3HLAND		11
#define SEARCH_HLAND		12
#define SEARCH_G			20
#define SEARCH_PLATEAU		32
#define SEARCH_MASK_PLATEAU	31

const float 		EPSILON 			= 0.002f;
const unsigned int 	MAX_UNSIGNED_INT 	= std::numeric_limits<unsigned int>::max();
const int32_t 		MAX_INT32 			= std::numeric_limits<int32_t>::max();
const float			FLOAT_INFINITY		= std::numeric_limits<float>::infinity();
const uint16_t		MAX_UINT16			= 65535;

typedef uint64_t	TMutex;
typedef uint32_t 	TOrdering;
typedef uint32_t 	TVarValue;
typedef uint16_t	TTimePoint;
typedef uint16_t	TStep;
typedef uint16_t	TVariable;
typedef uint16_t	TValue;

inline TTimePoint stepToStartPoint(TStep step) {	// Step number -> start time point
    return step << 1;
}

inline TTimePoint stepToEndPoint(TStep step) {		// Step number -> end time point
    return (step << 1) + 1;
}

inline TStep timePointToStep(TTimePoint t) {
    return t >> 1;
}

inline TTimePoint firstPoint(TOrdering ordering) {
    return ordering & 0xFFFF;
}

inline TTimePoint secondPoint(TOrdering ordering) {
    return ordering >> 16;
}

inline TOrdering getOrdering(TTimePoint p1, TTimePoint p2) {
    return (((TOrdering)p2) << 16) + p1;
}
#endif //GRSTAPS_UTILS_HPP
