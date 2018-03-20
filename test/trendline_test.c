#include "trendline.h"
#include <assert.h>
#include <math.h>

#define WND_SIZE			20
#define SMOOTHING			0.0
#define GAIN				1.0
#define AVGTIMEBETWEEN		10
#define PACKET_COUNT		(WND_SIZE * 2 + 1)

static inline double gaussian(double mean, double std)
{
	const double kPi = 3.14159265358979323846;
	double u1 = (double)(rand()) * 1.0 / 0xFFFFFFFF;
	double u2 = (double)(rand()) * 1.0 / 0xFFFFFFFF;
	return mean + std * sqrt(-2 * log(u1)) * cos(2 * kPi * u2);
}

static inline void EXPECT_NEAR(double src, double dst, double delta)
{
	assert(dst <= src + delta && dst >= src - delta);
}

static void trendline_func(double slope, double jitter_stdev, double toler)
{
	trendline_estimator_t* est;

	int64_t send_times[PACKET_COUNT];
	int64_t recv_times[PACKET_COUNT];
	int64_t send_start_time = rand() / 100000;
	int64_t recv_start_time = rand() / 100000;

	est = trendline_create(WND_SIZE, SMOOTHING, GAIN);

	for (size_t i = 0; i < PACKET_COUNT; ++i) {
		send_times[i] = send_start_time + i * AVGTIMEBETWEEN;
		double latency = i * AVGTIMEBETWEEN / (1 - slope);
		double jitter = gaussian(0, jitter_stdev);
		recv_times[i] = recv_start_time + latency + jitter;
	}

	for (size_t i = 1; i < PACKET_COUNT; ++i) {
		double recv_delta = recv_times[i] - recv_times[i - 1];
		double send_delta = send_times[i] - send_times[i - 1];
		trendline_update(est, recv_delta, send_delta, recv_times[i]);
		if (i < WND_SIZE)
			EXPECT_NEAR(trendline_slope(est), 0, 0.001);
		else
			EXPECT_NEAR(trendline_slope(est), slope, toler);
	}

	trendline_destroy(est);
}

void test_trendline()
{
	trendline_func(0.5, 0, 0.001);
	trendline_func(-1, 0, 0.001);
	trendline_func(0.5, AVGTIMEBETWEEN / 3.0, 0.01);
	trendline_func(-1, AVGTIMEBETWEEN / 3.0, 0.075);
}


