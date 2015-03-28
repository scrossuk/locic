Concurrency
===========

The standard library module 'std.concurrency' contains functionality to support concurrent execution of code.

Condition Variable
------------------

Condition variables are used for signalling concurrent execution tasks.

Here is an example:

.. code-block:: c++

	class ThreadSafeObject(std::mutex mutex, std::condition_variable condition, int value) {
		static create = default;
		
		void increment() {
			// (Need to mark unused.)
			unused auto lock = std::unique_lock(@mutex);
			@value++;
			@condition.notify_all();
		}
		
		void wait_for(const int value) {
			// (Need to mark unused.)
			unused auto lock = std::unique_lock(@mutex);
			while (@value != value) {
				@condition.wait(lock)
			}
		}
	}

Message Queue
-------------

Message queues are a safe mechanism for passing messages between two concurrent execution tasks, one of which is a 'reader' and one which is a 'writer'.

Here is an example:

.. code-block:: c++

	class ThreadSafeObject(std::message_queue<int> message_queue) {
		static create = default;
		
		void send_some(const int value) {
			@message_queue.send(value);
			@message_queue.send(value + 1);
			@message_queue.send(value * 2);
		}
		
		void wait_for(const int value) {
			auto wait_set = std::wait_set::edge_triggered();
			wait_set.insert(@message_queue.event_source());
			while (true) {
				if (@message_queue.empty()) {
					@wait_set.wait();
				} else {
					const int received_value = @message_queue.receive();
					if (received_value == value) {
						return;
					}
				}
			}
		}
	}

Mutex
-----

Mutexes ensure exclusive access by concurrent execution tasks to a sequence of code instructions.

Here is an example:

.. code-block:: c++

	class ThreadSafeObject(std::mutex mutex, int value) {
		static create = default;
		
		void modify(const int add) {
			// (Need to mark unused.)
			unused auto lock = std::unique_lock(@mutex);
			@value = @value * 2 + add;
		}
	}

Thread
------

Threads are pre-emptively scheduled concurrent (and potential parallel) execution tasks.

These can be managed with the std::concurrency::thread class; a support function called std::new_thread is available to facilitate creating threads.

Here is an example:

.. code-block:: c++
	
	class Task(int context) {
		static create = default;
		
		void run() {
			sleep(@context);
			printf(C"Done %d!\n", @context);
		}
	}
	
	void test() {
		// Template argument deduction will eliminate this redundancy
		// (to be implemented very soon).
		auto task2 = std::new_thread<Task>(Task(2));
		auto task4 = std::new_thread<Task>(Task(4));
		auto task6 = std::new_thread<Task>(Task(6));
		
		// Etc.
		
		task2.join();
		task4.join();
		// Thread destructor (for task6) automatically joins.
	}
