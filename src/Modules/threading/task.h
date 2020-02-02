#pragma once

#include "../generic/tupleunpacker.h"
#include "../generic/copy_traits.h"

namespace Engine
{
	namespace Threading
	{

		enum TaskState {
			YIELD,
			SUCCESS,
			FAILURE
		};

		struct Task {
			virtual TaskState execute() = 0;
			virtual std::unique_ptr<Task> clone() = 0;
			virtual ~Task() = default;
		};

		template <typename F>
		struct TaskImpl : Task {

			TaskImpl(F &&f) :
				mF(std::forward<F>(f))
			{
			}

			TaskState execute() override
			{
				return TupleUnpacker::invokeDefaultResult(SUCCESS, mF);
			}

			std::unique_ptr<Task> clone() override
			{
				return std::make_unique<TaskImpl<F>>(tryCopy(mF));
			}

		private:
			F mF;
		};

		struct TaskHandle {

			TaskHandle() = default;
			TaskHandle(TaskHandle &&) noexcept = default;

			template <typename F>
			TaskHandle(F &&f) :
				mTask(std::make_unique<TaskImpl<std::remove_reference_t<F>>>(std::forward<F>(f))) {}

			TaskHandle &operator=(TaskHandle &&) noexcept = default;

			TaskState operator()() const {
				return mTask->execute();
			}

			operator bool()
			{
				return mTask.operator bool();
			}

			TaskHandle clone() const
			{
				return *this;
			}

		protected:
			TaskHandle(const TaskHandle &other) :
				mTask(other.mTask->clone())
			{

			}

			TaskHandle &operator=(const TaskHandle &other)
			{
				mTask = other.mTask->clone();
				return *this;
			}

		private:
			std::unique_ptr<Task> mTask;
		};

	}
}