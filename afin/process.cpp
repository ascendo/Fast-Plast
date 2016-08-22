// $Author: afinit $
// $Date$
// $Log$
// Contains the Process class for afin

#include "process.hpp"
#include <thread>

int max_search_loops;
int contig_sub_len;
int extend_len;
int max_sort_char;
int min_cov;
int min_overlap;
int max_threads;
int initial_trim;
int max_missed;
double stop_ext;
bool test_run;
int print_fused;
int screen_output;
int log_output;
int verbose;
int no_fusion;
double mismatch_threshold;
std::mutex log_mut;

////////////////////////////////////
//////// PROCESS DEFINITIONS ///////
////////////////////////////////////

// Process constructor
Process::Process(){
  outfile = "afin_out";
  readsfiles = "";
  contigsfiles = "";
  reads = 0;
  contigs = 0;
  fuse = 0;
  max_iterations = 1;

  // initialize unordered_map for iterable options
  iterable_opts["max_search_loops"] = "10";
  iterable_opts["contig_sub_len"] = "100";
  iterable_opts["extend_len"] = "40";
  iterable_opts["max_sort_char"] = "4";
  iterable_opts["min_cov"] = "3";
  iterable_opts["min_overlap"] = "20";
  iterable_opts["initial_trim"] = "0";
  iterable_opts["max_missed"] = "5";
  iterable_opts["stop_ext"] = "0.5";
  iterable_opts["mismatch_threshold"] = "0.1";

  // Declare vector variables for storing iterable_opts
  std::vector<int> max_search_loops_iter;
  std::vector<int> contig_sub_len_iter;
	std::vector<int> extend_len_iter;
	std::vector<int> max_sort_char_iter;
	std::vector<int> min_cov_iter;
	std::vector<int> min_overlap_iter;
	std::vector<int> initial_trim_iter;
	std::vector<int> max_missed_iter;
	std::vector<double> stop_ext_iter;
	std::vector<double> mismatch_threshold_iter;
}

Process::~Process(){
  delete fuse;
  delete reads;
  delete contigs;
}

// initalize logfile
void Process::logfile_init(){
  Log::Inst()->open_log( outfile + ".log" );

  // output starting option values
  Log::Inst()->log_it( "OPTION VALUES" );
  Log::Inst()->log_it( "contig_sub_len: " + std::to_string(contig_sub_len) );
  Log::Inst()->log_it( "extend_len: " + std::to_string(extend_len) );
  Log::Inst()->log_it( "max_search_loops: " + std::to_string(max_search_loops) );
  Log::Inst()->log_it( "max_sort_char: " + std::to_string(max_sort_char) );
  Log::Inst()->log_it( "min_cov: " + std::to_string(min_cov) );
  Log::Inst()->log_it( "min_overlap: " + std::to_string(min_overlap) );
  Log::Inst()->log_it( "initial_trim: " + std::to_string(initial_trim) );
  Log::Inst()->log_it( "max_missed: " + std::to_string(max_missed) );
  Log::Inst()->log_it( "mismatch_threshold: " + std::to_string(mismatch_threshold) );
  Log::Inst()->log_it( "max_threads: " + std::to_string(max_threads) );
  Log::Inst()->log_it( "stop_ext: " + std::to_string(stop_ext) );
}

// Parse option int
void Process::parse_option( std::string opt_key, std::vector<int>* iter_vect ){
  std::string opt = iterable_opts[opt_key];

  try{
		// check for commas in string
		if( ! opt.find(',') ){
      iter_vect->push_back(std::stoi(opt));
    }
		else{
			// split along commas if found
			std::stringstream ss(opt);
			std::string item;
			while( getline( ss, item, ',' )){
				iter_vect->push_back(std::stoi(item));
			}
		}
	}
	catch( std::exception const & e ){
		Log::Inst()->log_it( "Error: " + std::string(e.what(), strlen(e.what())) + " : Invalid values: " + opt + " For option: " + opt_key );
		exit(0);
	}
}

// Parse option double
void Process::parse_option( std::string opt_key, std::vector<double>* iter_vect ){
  std::string opt = iterable_opts[opt_key];

	try{
		// check for commas in string
		if( ! opt.find(',') ){
			iter_vect->push_back(std::stof(opt));
		}
		else{
			// split along commas if found
			std::stringstream ss(opt);
			std::string item;
			while( getline( ss, item, ',' )){
				iter_vect->push_back(std::stof(item));
			}
		}
	}
	catch( std::exception const & e ){
		Log::Inst()->log_it( "Error: " + std::string(e.what(), strlen(e.what())) + " : Invalid values: " + opt + " For option: " + opt_key );
		exit(0);
	}
}

// Populate iterable options vector
void Process::populate_iterables(){
  // process iterable options
  parse_option( "max_search_loops", &max_search_loops_iter );
  parse_option( "contig_sub_len", &contig_sub_len_iter );
  parse_option( "extend_len", &extend_len_iter );
  parse_option( "max_sort_char", &max_sort_char_iter );
  parse_option( "min_cov", &min_cov_iter );
  parse_option( "min_overlap", &min_overlap_iter );
  parse_option( "initial_trim", &initial_trim_iter );
  parse_option( "max_missed", &max_missed_iter );
  parse_option( "stop_ext", &stop_ext_iter );
  parse_option( "mismatch_threshold", &mismatch_threshold_iter );

}

// set iterables global values at each iteration
// NOTE:: rebuild to check for length of vector
void Process::set_iterables( int i ){
  // max_search_loops_iter
  if( max_search_loops_iter.size() > i ){
  	max_search_loops = max_search_loops_iter[i];
  }
  else{
    max_search_loops = max_search_loops_iter.back();
  }

  // contig_sub_len_iter
  if( contig_sub_len_iter.size() > i ){
		contig_sub_len = contig_sub_len_iter[i];
  }
  else{
    contig_sub_len = contig_sub_len_iter.back();
  }

  // extend_len_iter
  if( extend_len_iter.size() > i ){
		extend_len = extend_len_iter[i];
  }
  else{
    extend_len = extend_len_iter.back();
  }

  // max_sort_char_iter
  if( max_sort_char_iter.size() > i ){
		max_sort_char = max_sort_char_iter[i];
  }
  else{
    max_sort_char = max_sort_char_iter.back();
  }

  // min_cov_iter
  if( min_cov_iter.size() > i ){
		min_cov = min_cov_iter[i];
  }
  else{
    min_cov = min_cov_iter.back();
  }

  // min_overlap_iter
  if( min_overlap_iter.size() > i ){
		min_overlap = min_overlap_iter[i];
  }
  else{
    min_overlap = min_overlap_iter.back();
  }

  // initial_trim_iter
  if( initial_trim_iter.size() > i ){
		initial_trim = initial_trim_iter[i];
  }
  else{
    initial_trim = initial_trim_iter.back();
  }

  // max_missed_iter
  if( max_missed_iter.size() > i ){
		max_missed = max_missed_iter[i];
  }
  else{
    max_missed = max_missed_iter.back();
  }

  // stop_ext_iter
  if( stop_ext_iter.size() > i ){
		stop_ext = stop_ext_iter[i];
  }
  else{
    stop_ext = stop_ext_iter.back();
  }

  // mismatch_threshold_iter
  if( mismatch_threshold_iter.size() > i ){
		mismatch_threshold = mismatch_threshold_iter[i];
  }
  else{
    mismatch_threshold = mismatch_threshold_iter.back();
  }
}

// Initializes data structures and turns over control to run_manager()
void Process::start_run(){
  // initialize logfile if logging enabled
  if( log_output || screen_output ){
    logfile_init();
  }

	// process iterable options
	populate_iterables();

  // import reads once
  reads = new Readlist( readsfiles );

  // loop through iterated options if present
	for( int i=0; i < max_iterations; i++ ){
		// prevent printing of fused contigs
		if( no_fusion )
			print_fused = 0;

    // Set options for this iteration
    set_iterables(i);

		// log output file
		Log::Inst()->log_it( std::string("output file: ") + outfile );

		// initialize objects
		contigs = new Contiglist( reads, contigsfiles, outfile );
		fuse = new Fusion( contigs, reads );

		Log::Inst()->log_it( "End initialization phase" );

		// make initial attempt to fuse contigs
		if( ! no_fusion )
			fuse->run_fusion( true );

		if( test_run )
			contigs->output_contigs( 0, outfile + ".fus", "mid" );

		run_manager();

    // write contigs to fasta
    contigs->create_final_fasta(i);
	}
}

// Manages run
void Process::run_manager(){
  // create thread array with max_thread entries
  std::vector<std::thread> t;
  Queue<int> qu;

  // loop max search loops
  for( int j=0; j<max_search_loops; j++ ){
    Log::Inst()->log_it( "Begin Extensions" );

    // initialize threads
    for( int i=0; i<max_threads; i++ ){
      t.push_back(std::thread( &Process::thread_worker, this, std::ref(qu), i ));
    }

    if( test_run ){
      Log::Inst()->log_it( "contigs.size(): " + std::to_string(contigs->get_list_size()) + " max_threads: " + std::to_string(max_threads) );
    }

    // push each thread onto queue
    for( int i=0; i<contigs->get_list_size(); i++ ){
      qu.push( i );
    }

    // push stop signals onto queue for each thread
    for( int i=0; i<max_threads; i++ ){
      qu.push( -1 );
    }

    // join threads
    for( int i=0; i<max_threads; i++ ){
      t[i].join();
    }

    // remove threads from vector
    t.erase( t.begin(), t.begin()+max_threads );

    // removed for master branch until algorithm can be adjusted
    if( ! no_fusion )
      fuse->run_fusion( false );

    if( test_run ){
      contigs->output_contigs( 0, outfile + ".fus" + std::to_string(j), "mid" );
    }
  }
}

// Consume function which will act as the driver for an individual thread
void Process::thread_worker( Queue<int>& q, unsigned int id) {
  for (;;) {
    auto item = q.pop();
    if( item == -1 ){
      break;
    }
    else{
      contigs->get_contig_ref(item)->extend( false );
      contigs->get_contig_ref(item)->extend( true );
    }
  }
}

//////////////////////////
// END PROCESS ///////////
//////////////////////////
