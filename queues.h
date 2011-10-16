#ifndef QUEUES_H
#define QUEUES_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <algorithm>
#include <set>

/* Priority queue that maintains strict ordering of its elements according to
 * their priority.
 */
template<typename T, typename Compare = std::less<T> >
class StrictPriorityQueue {

	typedef std::multiset<T, Compare> Queue;

	Queue queue; // Ordered low to high priority.

	public:

		typedef typename Queue::const_iterator const_iterator;

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

		const_iterator begin() const { return queue.begin(); }

		const_iterator end() const { return queue.end(); }

};

template<typename T, typename P>
struct PriorityItem {
	T item;
	P priority;
	PriorityItem(T const &item, P priority) : item(item), priority(priority) { }
};

template<typename T, typename P>
bool operator<(PriorityItem<T, P> const &a, PriorityItem<T, P> const &b) {
	return a.priority < b.priority;
}

/* Priority queue that needs explicitly provided priority values.
 */
template<
	typename T,
	typename P = float,
	typename QueueType = StrictPriorityQueue<PriorityItem<T, P> >
>
class ExplicitPriorityQueue {

	typedef PriorityItem<T, P> Item;

	class ConstIterator {
		typename QueueType::const_iterator i;
		public:
			ConstIterator(typename QueueType::const_iterator i) : i(i) { }
			ConstIterator &operator++() { ++i; return *this; }
			ConstIterator operator++(int) { ConstIterator old = *this; ++*this; return old; }
			bool operator==(ConstIterator const &other) const { return i == other.i; }
			bool operator!=(ConstIterator const &other) const { return !(*this == other); }
			T const &operator*() const { return i->item; }
	};

	QueueType queue;

	public:

		typedef ConstIterator const_iterator;

		bool empty() const { return queue.empty(); }

		size_t size() const { return queue.size(); }

		T const &front() const { return queue.front().item; }

		T const &back() const { return queue.back().item; }

		P front_priority() const { return queue.front().priority; }

		P back_priority() const { return queue.back().priority; }

		void pop_front() { queue.pop_front(); }

		void pop_back() { queue.pop_back(); }

		void insert(T const &t, P priority = P()) {
			queue.insert(Item(t, priority));
		}

		const_iterator begin() const { return ConstIterator(queue.begin()); }

		const_iterator end() const { return ConstIterator(queue.end()); }

};

/* Priority queue that computes the priorities of its elements using a supplied
 * priority function. This function can be changed to cause updating of all
 * priorities.
 */
template<typename T, typename P = float, typename QueueType = ExplicitPriorityQueue<T, P> >
class DynamicPriorityQueue {

	public:

		typedef boost::function<P(T const &)> PriorityFunction;

	private:

		PriorityFunction priorityFunction;
		QueueType queue;

	public:

		DynamicPriorityQueue(PriorityFunction const &priorityFunction = PriorityFunction())
		:
			priorityFunction(priorityFunction)
		{
		}

		P priority(T const &t) const {
			return priorityFunction ? priorityFunction(t) : P();
		}

		void setPriorityFunction(PriorityFunction const &priorityFunction) {
			this->priorityFunction = priorityFunction;
			recompute();
		}

		bool empty() const { return queue.empty(); }

		size_t size() const { return queue.size(); }

		T const &front() const { return queue.front(); }

		T const &back() const { return queue.back(); }

		P front_priority() const { return queue.front_priority(); }

		P back_priority() const { return queue.back_priority(); }

		void pop_front() { queue.pop_front(); }

		void pop_back() { queue.pop_back(); }

		void insert(T const &t) {
			queue.insert(t, priority(t));
		}

	private:

		void recompute() {
			QueueType orig;
			std::swap(orig, queue);
			for (typename QueueType::const_iterator i = orig.begin(); i != orig.end(); ++i) {
				insert(*i);
			}
		}
};

#endif
