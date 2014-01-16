
#include <errno.h>

#include "fileinfo.h"
#include "appoption.h"

int app_file_debug = 0;						// Global FILE debug flag. Declared in fileinfo.h

//
// FileInfo functions
//

/**
* Initializes a FileInfo buffer with the given
* information. This function can be used with a
* statically allocated buffer.
*
* @param finfo		the FileInfo buffer to initialize
* @param pname		the path name of the file
*			(hopefully the full path)
* @param statinfo	the stat structure for the 
*			file
*/
void fileinfo_init(FileInfo *finfo, char *pname, struct stat *statinfo) {
	bzero(finfo,sizeof(FileInfo));				// clear out the buffer
	finfo->type = _FILEINFO_TYPE_;				// set the type
	if(pname) strcpy(finfo->path,pname);			// copy the path name
	if(statinfo) {						// copy the stat structure - if given 
	  memcpy(&finfo->st,statinfo,sizeof(struct stat));
	  finfo->exists = (finfo->st.st_ctime > 0);		// if create time populated -> file exists
	}
	else
	  finfo->exists = (-1);					// no stat information

	return;
}

/**
* Creates a new FileInfo buffer.
*
* @param pname		the path name of the file
*			(hopefully the full path)
* @param statinfo	the stat structure for the 
*			file
*
* @return a new FileInfo buffer
*/
FileInfo *fileinfo_new(char *pname, struct stat *statinfo) {
	FileInfo *newbuf = (FileInfo *)malloc(sizeof(FileInfo));

	fileinfo_init(newbuf,pname,statinfo);			// initalize the new buffer
	return(newbuf);
}

/**
* Deallocates a FileInfo buffer.
*
* @param finfo		the buffer to deallocate
*
* @return a NULL FileInfo pointer
*/
FileInfo *fileinfo_del(FileInfo *finfo) {
	if(finfo) free(finfo);
	return((FileInfo *)NULL);
}

/**
* Tests to see if a file is potentially
* big enough to chunk. It is assumed that 
* the FileInfo structure passed in has a 
* populated stat structure - though this 
* function will handle the case where the
* stat structure is empty.
*
* This function tests the file size against
* a the chunkat limit set in the AppOptions
* structure.
*
* @oaram finfo		the FileInfo structure to test
* @param app_o		the AppOptions structure containing
* 			the application option values
*
* @return 1 is returned if the file can be chunked. Otherwise
* 	0 is returned - including inthe case of an empty 
* 	FileInfo structure or stat buffer
*/
int fileinfo_isChunkable(FileInfo *finfo, AppOptions *app_o) {
	return(finfo && finfo->st.st_size >= app_o->start_chunking);
}

/**
* Populates the stat structure via a stat() call.
* This function uses lstat() to do so. If an ENOENT
* or no error occures from the lstat(), then the 
* exists flag for this structure is set appropeiately.
* If any other error is generated by the lstat(), then
* a non-zero result is returned.
*
* @oaram finfo		the FileInfo structure to populate
*
* @return 0 if there are no problems running the stat() call.
* 	If lstat() cannot be called (i.e. no path specified),
* 	then a negative value is returned. If there is an error
* 	with the lstat() call, a positive value is returned, which
* 	should correspond to the errno.
*/
int fileinfo_lstat(FileInfo *finfo) {
	int err = 0;

	if(finfo->type != _FILEINFO_TYPE_) return(-1);		// if type not set, then FileInfo structure not initialize (or not a FileInfo!)
	if(lstat(finfo->path, &finfo->st) == -1) {
	  if(errno != ENOENT) 
	    err = errno;
	  else
	    finfo->exists = 0;
	}
	else
	  finfo->exists = 1;

	return(err);
}

//
// FilePair functions
//

/**
* Creates a new FilePair buffer.
*
* @param src	a FileInfo structure that 
*		describes the source file
* @param dest	a FileInfo structure that 
*		describes the destination file
*
* @return a new FilePair buffer
*/
FilePair *filepair_new(FileInfo *src, FileInfo *dest) {
	FilePair *newbuf = (FilePair *)malloc(sizeof(FilePair));

	bzero(newbuf,sizeof(FilePair));				// clear out the buffer
	newbuf->type = _FILEPAIR_TYPE_;				// set the type
	newbuf->chunknum = (-1L);				// initially not chunked.
	if(src) memcpy(&newbuf->src,src,sizeof(FileInfo));	// copy the src file info into the buffer
	if(dest) memcpy(&newbuf->dest,dest,sizeof(FileInfo));	// copy the dest file info into the buffer

	return(newbuf);
}

/**
* Deallocates a FilePair buffer.
*
* @param fpair		the buffer to deallocate
*
* @return a NULL FilePair pointer
*/
FilePair *filepair_del(FilePair *fpair) {
	if(fpair) free(fpair);
	return((FilePair *)NULL);
}

//
// FileWorkStat functions
//

/**
* Initializes a FileWorkStat buffer with the given
* information. This function can be used with a
* statically allocated buffer.
*
* @param fstat		the FileWorkStat buffer to initialize
* @param fpair		a FilePair structure that holds the information
* 			about the files that have been (or need to be)
* 			processed
* @param perf		holds performance information regarding the
* 			processing of the files in the FilePair 
* 			structure 
*/
void fileworkstat_init(FileWorkStat *fstat, FilePair *fpair, PerfStat *perf) {
	bzero(fstat,sizeof(FileWorkStat));			// clear out the buffer
	fstat->type = _FILEWORKSTAT_TYPE_;			// set the type
	if(fpair) 						// copy the FilePair structure - if given 
	  memcpy(&fstat->files,fpair,sizeof(FilePair));
	if(perf) 						// copy the PerfStat structure - if given 
	  memcpy(&fstat->perf,perf,sizeof(PerfStat));

	return;
}

/**
* Creates a new FileWorkStat buffer.
*
* @param fpair		a FilePair structure that holds the information
* 			about the files that have been (or need to be)
* 			processed
* @param perf		holds performance information regarding the
* 			processing of the files in the FilePair 
* 			structure 
*
* @return a new FileWorkStat buffer
*/
FileWorkStat *fileworkstat_new(FilePair *fpair, PerfStat *perf) {
	FileWorkStat *newbuf = (FileWorkStat *)malloc(sizeof(FileWorkStat));

	fileworkstat_init(newbuf,fpair,perf);			// initalize the new buffer
	return(newbuf);
}

/**
* Deallocates a FileWorkStat buffer.
*
* @param fstat		the buffer to deallocate
*
* @return a NULL FileWorkStat pointer
*/
FileWorkStat *fileworkstat_del(FileWorkStat *fstat) {
	if(fstat) free(fstat);
	return((FileWorkStat *)NULL);
}

//
// WorkItem Support functions
//

/**
* Attach or assign a FileInfo structure to a WorkItem.
* If item is NULL or fpair is NULL, then nothing is done.
* Uses workitem_update_work().
*
* @param finfo	the FileInfo structure to
*		update with
* @param item	the WorkItem to update
*/
void fileinfo_attach(FileInfo *finfo, WorkItem *item) {
	workitem_update_work(item,(void *)finfo,sizeof(FileInfo));
	return;
}

/**
* A function to return a work item's file info structure.
* If the buffer pointer is NULL, then the work
* item does not have a file info structure. Uses
* workitem_get_work().
*
* @param item		the item to reference
*
* @return the pointer to the item's file info structure
*/
FileInfo *workitem_get_fileinfo(WorkItem *item) {
	return((FileInfo *)workitem_get_work(item));
}

/**
* A function to test if a work item has an allocated
* FileInfo structure.
*
* @param item		the item to test
*
* @return non-zero if a FileInfo structure is allocated.
*	zero is returned otherwise, including if the
*	item is NULL.
*/
int workitem_hasFileinfo(WorkItem *item) {
	if(item) return(item->workdata && ((FileInfo *)item->workdata)->type == _FILEINFO_TYPE_);
	return(0);
}

/**
* Attach or assign a FilePair structure to a WorkItem.
* If item is NULL or fpair is NULL, then nothing is done.
* Uses workitem_update_work().
*
* @param fpair	the FilePair structure to
*		update with
* @param item	the WorkItem to update
*/
void filepair_attach(FilePair *fpair, WorkItem *item) {
	workitem_update_work(item,(void *)fpair,sizeof(FilePair));
	return;
}

/**
* A function to return a work item's file pair buffer.
* If the buffer pointer is NULL, then the work
* item does not have a file pair buffer. Uses
* workitem_get_work().
*
* @param item		the item to reference
*
* @return the pointer to the item's file pair buffer
*/
FilePair *workitem_get_filepair(WorkItem *item) {
	return((FilePair *)workitem_get_work(item));
}

/**
* A function to test if a work item has an allocated
* FilePair structure.
*
* @param item		the item to test
*
* @return non-zero if a FilePair structure is allocated.
*	zero is returned otherwise, including if the
*	item is NULL.
*/
int workitem_hasFilepair(WorkItem *item) {
	if(item) return(item->workdata && ((FilePair *)item->workdata)->type == _FILEPAIR_TYPE_);
	return(0);
}

/**
* Attach or assign a FileWorkStat structure to a WorkItem.
* If item is NULL or fpair is NULL, then nothing is done.
* Uses workitem_update_work().
*
* @param fstat	the FileWorkStat structure to
*		update with
* @param item	the WorkItem to update
*/
void fileworkstat_attach(FileWorkStat *fstat, WorkItem *item) {
	workitem_update_work(item,(void *)fstat,sizeof(FileWorkStat));
	return;
}

/**
* A function to return a work item's file info structure.
* If the buffer pointer is NULL, then the work
* item does not have a FileWorkStat structure. Uses
* workitem_get_work().
*
* @param item		the item to reference
*
* @return the pointer to the item's file info structure
*/
FileWorkStat *workitem_get_fileworkstat(WorkItem *item) {
	return((FileWorkStat *)workitem_get_work(item));
}

/**
* A function to test if a work item has an allocated
* FileWorkStat structure.
*
* @param item		the item to test
*
* @return non-zero if a FileWorkStat structure is allocated.
*	zero is returned otherwise, including if the
*	item is NULL.
*/
int workitem_hasFileworkstat(WorkItem *item) {
	if(item) return(item->workdata && ((FileWorkStat *)item->workdata)->type == _FILEWORKSTAT_TYPE_);
	return(0);
}
