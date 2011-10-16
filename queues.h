#ifndef QUEUES_H
#define QUEUES_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <set>

/* Priority queue that maintains strict ordering of its elements according to
 * their priority.
 */
template<typename T, typename Compare = std::less<T> >
class StrictPriorityQueue {

	typedef std::multiset<T, Compare> Queue;

	Queue queue; // Ordered low to high priority.

	public:

		bool empty() const { return queue.empty(); }

		size_t size() const { return queue.size(); }

		void clear() { queue.clear(); }

		T const &front() const { return *queue.rbegin(); }

		T const &back() const { return *queue.begin(); }

		void pop_front() {
			typename Queue::iterator i = queue.end();
			--i;
			queue.erase(i);
		}

		void pop_back() { queue.erase(queue.begin()); }

		void insert(T const &t) { queue.insert(t); }

};

/* Priority queue that needs explicitly provided priority values.
 */
template<typename T, typename Priority = float, template<typename T, typename Compare> class QueueType = StrictPriorityQueue>
class ExplicitPriorityQueue {

	typedef std::pair<T, Priority> Pair;
	struct Compare {
		bool operator()(Pair const &a, Pair const &b) const {
			return a.second < b.second;
		}
	};
	typedef QueueType<Pair, Compare> Queue;

	Queue queue;

	public:

		bool empty() const { return queue.empty(); }

		size_t size() const { return queue.size(); }

		T const &front() const { return queue.front().first; }

		T const &back() const { return queue.back().first; }

		void pop_front() { queue.pop_front(); }

		void pop_back() { queue.pop_back(); }

		void insert(T const &t, Priority priority = Priority()) {
			queue.insert(Pair(t, priority));
		}
};

/* Priority queue that computes the priorities of its elements using a supplied
 * priority function. This function can be changed to cause updating of all
 * priorities.
 */
template<typename T, typename Priority = float, template<typename T, typename Priority> class QueueType = ExplicitPriorityQueue>
class ComputingPriorityQueue {

	public:

		typedef boost::function<Priority(T const &)> PriorityFunction;

	private:

		typedef QueueType<T, Priority> Queue;

		PriorityFunction priorityFunction;
		Queue queue;

	public:

		ComputingPriorityQueue(PriorityFunction const &priorityFunction = PriorityFunction())
		:
			priorityFunction(priorityFunction)
		{
		}

		void setPriorityFunction(PriorityFunction const &priorityFunction) {
			this->priorityFunction = priorityFunction;
			recompute();
		}

		bool empty() const { return queue.empty(); }

		size_t size() const { return queue.size(); }

		T const &front() const { return queue.front().first; }

		T const &back() const { return queue.back().first; }

		void pop_front() { queue.pop_front(); }

		void pop_back() { queue.pop_back(); }

		void insert(T const &t) {
			Priority priority = priorityFunction ? priorityFunction(t) : Priority();
			queue.insert(Pair(t, priority));
		}

	private:

		void recompute() {
			Queue orig;
			queue.swap(orig);
			for (typename Queue::const_iterator i = orig.begin(); i != orig.end(); ++i) {
				insert(i->first);
			}
		}
};

#endif
