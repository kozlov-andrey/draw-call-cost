#include "CollectStatistics.h"

#include "Time.h"
#include "GLRenderer.h"

#include <iostream>
#include <fstream>
#include <map>

void collect_statistics(std::function<void()> swap_func, int width, int height, const char *output_path)
{
	GLRenderer renderer(width, height);

	const auto policies = { ChangeStatePolicy::DontChange, ChangeStatePolicy::ChangeOnlyProgram, ChangeStatePolicy::DontChangeProgram, ChangeStatePolicy::Change };
		
	struct MillionTriPerSecEntry final
	{
		unsigned tri_per_batch = 0;
		unsigned draw_call_count = 0;
		double million_tri_per_sec = 0;
	};
	struct BatchPerSecEntry final
	{
		unsigned tri_per_batch = 0;
		double thosands_batch_per_sec = 0.;
	};

	std::map<ChangeStatePolicy, std::vector<MillionTriPerSecEntry>> tri_per_sec_statistics;
	std::map<ChangeStatePolicy, std::vector<BatchPerSecEntry>> batch_per_sec_statistics;

	std::ofstream file(std::string(output_path) + "statistics.txt");

#ifdef __ANDROID__
	const auto tri_per_frame = 100000u;
#else
	const auto tri_per_frame = 1000000u;
#endif
	const unsigned tri_per_sec_points[] = { 10, 30, 50, 70, 90, 110, 130, 150, 170, 190, 300, 500, 700, 900, 1100, 1300, 1500, 5000, 10000,
		20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, tri_per_frame };
	const unsigned batch_per_sec_points[] = { 10, 50, 100, 250, 500, 1000, 2000, 3000, 4000, 5000, 10000 };

	for (const auto policy : policies)
	{
		{
			const auto frame_count = 1000;

			for (const auto tri_per_batch : tri_per_sec_points)
			{
				renderer.Setup(tri_per_batch, tri_per_frame, policy);
				glFinish();
				const auto time_start = time_now();
				for (int i = 0; i < frame_count; ++i)
				{
					renderer.Draw();
					// To deal with vsync. Doesn't change trends much.
					glFinish();
				}
				swap_func();
				const auto time_end = time_now();

				MillionTriPerSecEntry entry;
				entry.tri_per_batch = tri_per_batch;
				entry.draw_call_count = tri_per_frame / tri_per_batch;
				const auto million_triangles_count = ((tri_per_frame / tri_per_batch) * tri_per_batch) / 1000000. * frame_count;
				entry.million_tri_per_sec = million_triangles_count / ((time_end - time_start) / 1000000000.);
				tri_per_sec_statistics[policy].push_back(entry);
			}
		}

		{
			const auto frame_count = 1000;
#ifdef __ANDROID__
			const auto batch_per_frame = 500;
#else
			const auto batch_per_frame = 4000;
#endif

			for (const auto tri_per_batch : batch_per_sec_points)
			{
				renderer.Setup(tri_per_batch, tri_per_batch * batch_per_frame, policy);
				const auto time_start = time_now();
				for (int i = 0; i < frame_count; ++i)
				{
					renderer.Draw();
					// To deal with vsync. Doesn't change trends much.
					glFinish();
				}
				swap_func();
				const auto time_end = time_now();

				BatchPerSecEntry entry;
				entry.tri_per_batch = tri_per_batch;
				entry.thosands_batch_per_sec = (frame_count / 1000.f * batch_per_frame) / ((time_end - time_start) / 1000000000.);
				batch_per_sec_statistics[policy].push_back(entry);
			}
		}
	}

	file << "Tri_per_sec_statistics" << std::endl;

	file << "Tri_per_batch No_state_change Change_only_program No_program_change Change_state" << std::endl;
	int i = 0;
	for (const auto &tri_per_batch : tri_per_sec_points)
	{
		file
			<< tri_per_batch
			<< " " << tri_per_sec_statistics[ChangeStatePolicy::DontChange][i].million_tri_per_sec
			<< " " << tri_per_sec_statistics[ChangeStatePolicy::ChangeOnlyProgram][i].million_tri_per_sec
			<< " " << tri_per_sec_statistics[ChangeStatePolicy::DontChangeProgram][i].million_tri_per_sec
			<< " " << tri_per_sec_statistics[ChangeStatePolicy::Change][i].million_tri_per_sec 
			<< std::endl;
		++i;
	}

	i = 0;
	file << "Batch_per_sec_statistics" << std::endl;

	file << "Tri_per_batch No_state_change Change_only_program No_program_change Change_state" << std::endl;
	for (const auto &tri_per_batch : batch_per_sec_points)
	{
		file
			<< tri_per_batch
			<< " " << batch_per_sec_statistics[ChangeStatePolicy::DontChange][i].thosands_batch_per_sec
			<< " " << batch_per_sec_statistics[ChangeStatePolicy::ChangeOnlyProgram][i].thosands_batch_per_sec
			<< " " << batch_per_sec_statistics[ChangeStatePolicy::DontChangeProgram][i].thosands_batch_per_sec
			<< " " << batch_per_sec_statistics[ChangeStatePolicy::Change][i].thosands_batch_per_sec
			<< std::endl;
		++i;
	}
}

