/*
 * cthread.h
 *
 *  Created on: 26.10.2014
 *      Author: andreas
 */

#ifndef CTHREAD_H_
#define CTHREAD_H_

#include <pthread.h>

#include <rofl/common/ciosrv.h>
#include <rofl/common/croflexception.h>
#include <rofl/common/logging.h>

namespace rofl {
namespace common {

class eRofThreadBase : public RoflException {
public:
	eRofThreadBase(const std::string& __arg) : RoflException(__arg) {};
};
class eRofThreadFailed : public eRofThreadBase {
public:
	eRofThreadFailed(const std::string& __arg) : eRofThreadBase(__arg) {};
};

class cthread {
public:

	/**
	 *
	 */
	cthread() : pid(0), result(0) {};

	/**
	 *
	 */
	virtual
	~cthread() {};

public:

	/**
	 *
	 */
	void
	set_pid(pthread_t pid) { this->pid = pid; };

	/**
	 *
	 */
	pthread_t
	get_pid() const { return pid; };

	/**
	 *
	 */
	void
	set_result(int result) { this->result = result; };

	/**
	 *
	 */
	const int&
	get_result() const { return result; };

protected:

	/**
	 *
	 */
	virtual void
	init_thread() {}; // to be overwritten by derived class

	/**
	 *
	 */
	virtual void
	release_thread() {}; // to be overwritten by derived class

	/**
	 *
	 */
	void
	start_thread(const pthread_attr_t* attr = (const pthread_attr_t*)0) {
		rofl::logging::debug << "[rofl-common][cthread][start_thread] create" << std::endl;
		int rc = pthread_create(&pid, attr, cthread::run_thread, (void*)this);
		rofl::logging::debug << "[rofl-common][cthread][start_thread] create done tid: 0x" << pid << std::endl;
		if (rc != 0) {
			switch (rc) {
			case EAGAIN: {
				throw eRofThreadFailed("cthread::start_thread() insufficient resources");
			} break;
			case EINVAL: {
				throw eRofThreadFailed("cthread::start_thread() invalid thread attributes");
			} break;
			case EPERM: {
				throw eRofThreadFailed("cthread::start_thread() permission denied");
			} break;
			default: {
				throw eRofThreadFailed("cthread::start_thread() unknown error occured");
			};
			}
		}
	}

	/**
	 *
	 */
	void
	stop_thread() {
		rofl::logging::debug << "[rofl-common][cthread][stop_thread] cancel tid: 0x" << pid << std::endl;
		pthread_cancel(pid);
		void* retval;
		rofl::logging::debug << "[rofl-common][cthread][stop_thread] join tid: 0x" << pid << std::endl;
		int rc = pthread_join(pid, &retval);
		rofl::logging::debug << "[rofl-common][cthread][stop_thread] join done tid: 0x" << pid << std::endl;
		if (rc != 0) {
			switch (rc) {
			case EDEADLK: {
				throw eRofThreadFailed("cthread::stop_thread() -join- deadlock");
			} break;
			case EINVAL: {
				throw eRofThreadFailed("cthread::stop_thread() -join- thread not joinable");
			} break;
			case ESRCH: {
				//throw eRofThreadFailed("cthread::stop_thread() -join- thread not found");
			} break;
			default: {
				throw eRofThreadFailed("cthread::stop_thread() -join- unknown error");
			};
			}
		}
	};

private:

	/**
	 *
	 */
	static void*
	run_thread(void* arg) {
		cthread& thread = *(static_cast<cthread*>( arg ));

		thread.init_thread();

		rofl::cioloop::get_loop(thread.get_pid()).run();

		thread.release_thread();

		return (void*)&(thread.get_result());
	};

protected:

	pthread_t 		pid;
	int				result;
};

}; // end of namespace common
}; // end of namespace rofl

#endif /* CTHREAD_H_ */
