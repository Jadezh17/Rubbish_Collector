
#include "rc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Returns an allocation of n bytes and creates an internal rc entry.
 *
 * If the ptr argument is NULL and deps is NULL, it will return a new
 * allocation
 *
 * If the ptr argument is not NULL and an entry exists, it will increment
 *  the reference count of the allocation and return a strong_ref pointer
 *
 * If the ptr argument is NULL and deps is not NULL, it will return
 * a new allocation but the count will correlate to the dependency
 * if the dependency is deallocated the reference count on the object will
 * decrement
 *
 * If the ptr argument is not NULL and an entry exists and dep is not
 * NULL, it will increment the count of the strong reference but the count
 * will be related to the dependency, if the dependency is deallocated the
 * reference count on the object will decrement
 */


// static (struct strong_ref**)X/ reference_count_graph;


static int count = 0;
static struct strong_ref* ptr;
static struct strong_ref** reference_count_graph = &ptr;






// reference_count_graph = malloc(3);

struct strong_ref* rc_alloc(void* ptr, size_t n, struct strong_ref* dep)
{

	if(ptr == NULL && dep == NULL)
	{

		struct strong_ref *ref;
		if(count <30)
		{
			*reference_count_graph= (struct strong_ref*)malloc(sizeof(struct strong_ref)*30);
		}
		else
		{
			*reference_count_graph= realloc(*reference_count_graph,sizeof(struct strong_ref)*30);
		}
		ref = &(*reference_count_graph)[count];
		ref->ptr = calloc(10,sizeof(void*));
		ref->entry.count =0;
		ref->entry.count +=1;
		ref->entry.n_deps = 0;
		ref->entry.dep_capacity = 0;

		count +=1;


		return ref;
	}

	else if(ptr != NULL)
	{

		for(int i = 0; i < count; i++)
		{

			if(reference_count_graph[i]->ptr == ptr)
			{

				if(dep!= NULL)
				{
					reference_count_graph[i]->entry.count +=1;
				}
				else
				{
					reference_count_graph[i]->entry.count +=1;
					return reference_count_graph[i];
				}

			}
		}
	}


}


/**
 * Downgrades a strong reference to a weak reference, this will decrement the
 * reference count by 1
 * If ref is NULL, the function will return an invalid weak ref object
 * If ref is a value that does not exist in the reference graph, it will return
 * an weak_ref object that is invalid
 *
 * If ref is a value that does exist in the reference graph, it will return
 *    a valid weak_ref object
 *
 * An invalid weak_ref object is where its entry_id field is set to
 *   0xFFFFFFFFFFFFFFFF
 *
 * @param strong_ref* ref (reference to allocation)
 * @return weak_ref (reference with an entry id)
 */
struct weak_ref rc_downgrade(struct strong_ref* ref)
{

	struct weak_ref r = { 0xFFFFFFFFFFFFFFFF };

	if(ref == NULL)
	{
		return r;
	}
	for(int j = 0; j< count; j++)
	{
		reference_count_graph[j]->entry.count -=1;
		if(reference_count_graph[j]->ptr == ref->ptr)
		{
			if(reference_count_graph[j]->entry.count == 0)
			{
				free(ref->ptr);
				free(reference_count_graph[j]);

				return r;
			}
			else
			{
				struct weak_ref found = {ref->ptr};
				return found;
			}
		}
	}


}


/**
 * Upgrdes a weak reference to a strong reference.
 * The weak reference should check that the entry id is valid (bounds check)
 * If a strong reference no longer exists or has been deallocated, the return
 *   result should be null.
 */
struct strong_ref* rc_upgrade(struct weak_ref ref)
{
	if(count>0 && ref.entry_id != 0xFFFFFFFFFFFFFFFF )
	{
		for(int i = 0; i < count ; i++)
		{
			if(ref.entry_id == reference_count_graph[i]->ptr)
			{

				reference_count_graph[i]->entry.count +=1;
				return reference_count_graph[i];
			}
			else
			{
				return NULL;
			}
		}
	}
	else
	{

		return NULL;
	}
}


/**
 * Cleans up the reference counting graph.
 */
void rc_cleanup()
{
	for(int i = 0 ; i < count; i++)
	{

		if(reference_count_graph[i] != NULL)
		{
			reference_count_graph[i]->entry.count = 0;
			free(reference_count_graph[i]->ptr);

			if(reference_count_graph[i]->entry.n_deps !=0)
			{
				free(reference_count_graph[i]->entry.dep_list);
			}
			reference_count_graph[i]->entry.n_deps = 0;
			reference_count_graph[i]->entry.count = 0;
			free(reference_count_graph[i]);
		}
	}


}


// //
// int main()
// {
// 	int pass = 0;
//
//
// 	struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);
// 	struct strong_ref* a = rc_alloc(m->ptr, 0, NULL);
//
// 	*((int*)m->ptr) = 100;
//
//
// 	struct weak_ref w = rc_downgrade(m);
//
//
// 	struct strong_ref* p = rc_upgrade(w);
//
//
// 	if(p != NULL) {
// 			pass++;
// 	}
// 	//
// 	int v = *((int*)p->ptr);
// 	if(v == 100) {
// 		pass++;
// 	}
// 	rc_downgrade(p);
//
//
// 	w = rc_downgrade(a);
//
// 	if(w.entry_id == 0xFFFFFFFFFFFFFFFF){
// 		printf("weak ref\n");
// 	}
//
// 	p = rc_upgrade(w);
// 	//
// 	if(p == NULL) {
// 			pass++;
// 	}
// 	printf("pass: %d\n",pass);
// }
