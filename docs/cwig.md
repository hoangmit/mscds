<a id="intro"><h1>Introduction</h1></a>
CWig is a format and toolkit for storing and analysing genome-wide density signal data. CWig files use small space and provide fast access operations. It was developed as an alternative for bigWig format from UCSC. 
The project aims to give flexible and convenience tools to support visualization and analysis process.

<a id="install"><h1>Installation</h1></a>

<h2>Downloads</h2>

The binary executables of CWig tools are available for Windows (<a href= "files/cwig-win32.zip">win32</a>, <a href="files/cwig-win64.zip">win64</a>) and Linux (<a href= "files/cwig-linux-i686.tar.gz">i686</a>, <a href= "files/cwig-linux-x86_64.tar.gz">x86_64</a>).
CWig is an open source software. 

Source code is available at <a href="https://bitbucket.org/hmm01/mscds">bitbucket</a>
Other binary builds are provided upon request. (Please <a href="mailto:ksung@comp.nus.edu.sg?Subjection=CWig%20source%20code%20request" target="_top">e-mail</a> to us for the release candiate code.)


<h2>Requirements</h2>
CWig can be compiled and used in both Windows, Linux and MacOS. CWig's core features requires the following software and libraries:
C++11 compiler (e.g. gcc 4.6+), <a href="http://www.boost.org/">boost library</a>, <a href="http://www.cmake.org/">CMake</a>.
CWig's API for Python requires <a href="http://www.swig.org/">SWIG</a> and <a href="http://www.python.org/">Python</a> library.
CWig's API for Java requires <a href="http://www.swig.org/">SWIG</a> and <a href="http://www.java.com/">Java</a> JNI library.
bigWig2cwig tool requires <a href="http://genome-source.cse.ucsc.edu/gitweb/?p=kent.git;a=summary">UCSC jksrc library</a>.
Remote access using HTTPS feature requires <a href="https://www.openssl.org/">OpenSSL</a> library.
<!-- or http://hgdownload.cse.ucsc.edu/admin/ -->


<BR>

<h2>Build</h2>
Download and extract the source to a directory. From that directory, create a build directory, use cmake to generate a Makefile, and compile the package. For example:
<pre>
mkdir build
cd build
cmake ..
make
</pre>

If the process is successful, the excutables are produced at <code>./bin</code>, and the libraries are placed at <code>./lib</code>.
<br>

<a id="doc"><h1>Documentations</h1></a>

<h2>Command-line tools</h2>
<!-- <code>bedgraph2cwig</code>, <code>bigWig2cwig</code>, <code>cwig2bedgraph</code>, <code>cwigSummary</code>, <code>cwigSummaryBatch</code>, <code>cwigInfo</code>.<BR> -->

CWig provide the following command-line tools:
<ul>
<li>
<code>bedgraph2cwig</code>, <code>bigWig2cwig</code>, <code>cwig2bedgraph</code>: convert cwig to/from other formats.
</li>
<li>
<code>cwigSummary</code>, <code>cwigSummaryBatch</code>: query cwig file.
<li><code>cwigInfo</code>: checks cwig file.
</li>
</ul>

<h3>Create cwig file from bedgraph file</h3>
<pre>
# bedgraph2cwig (input_bedGraph_file) (output_cwig_file)
$ bedgraph2cwig data.bedGraph data.cwig
</pre>

<h3>Create cwig file from bigWig file</h3>
<pre>
# bedgraph2cwig (input_bedWig_file) (output_cwig_file)
$ bigWig2cwig data.bedWig data.cwig
</pre>

<h3>Decompress cwig file to bedGraph file</h3>
<pre>
# bedgraph2cwig (input_cwig_file) (output_bedGraph_file)
$ cwig2bedGraph data.cwig data.bedGraph
</pre>

Note that, we do not provide utility to convert cwig file to bigWig file directly. However, bedGraph file can be converted to bigWig file using UCSC jksrc tools. i.e. <code>bedGraphTobigWig</code> program.

<h3>Query cwig file</h3>
<code>cwigSummary</code> commandline tool allows user to query some regions in a cwig file.
The synatx for the command is:
<pre>
# cwigSummary [avg|cov|min|max|lst|map] (cwig_file)
#             (chromosome_name) (start_position) (end_position)
#             [window_count=1]
</pre>
The input file name is specified by <code>cwig_file</code> parameter. Note that the cwig_file can be located in a web-server e.g. http://yourdomain.com/path/data.cwig. 
The query regions are specified by four parameters: <code>chromosome_name</code>, <code>start_position</code>, <code>end_position</code>, and <code>window_count</code>. The utility divides the range from <code>start_position</code> to <code>end_position</code> from the chromosome into <code>window_count</code> regions. It performs the query in each region and returns the result.
For example:
<pre>
$ cwigSummary avg data.cwig chr1 100000 500000 4
0.00023571  0.000276345  0.000120334  9.97036e-05
</pre>
The first number is the average of the values in the file for the region from [100000, 200000) in chromosome "chr1". The second number is the average for the range [200000, 30000), etc.<BR>

<BR>
The supported query are:
<ul>
<li><code>avg</code> query: returns average signals value for each region.
</li>
<li><code>cov</code> query: returns the coverage (percentage of the bases that have data) for each region.
</li>
<li><code>min</code> query: returns the minimal signal value for each region.
</li>
<li><code>max</code> query: returns the maximal signal value for each region.
</li>
<li><code>lst</code> query: returns a list of bedGraph intervals that are intersect <code>start_position</code> to <code>end_position</code> region. (<code>window_count</code> value is ignore.)
</li>
<li><code>map</code> query: returns the signal values (for each base) inside the region. (<code>window_count</code> value is ignore.)
</ul>



<BR>
More examples:<BR>
<!--
The data.cwig file is constructed from wgEncodeOpenChromChipProgfibCtcfSig dataset in UCSC Encode.
-->
<code>avg</code> example:
<pre>
$ cwigSummary avg data.cwig chr1 100000 500000 4
0.0174  0.0784  0.02679  0.01059
</pre>

<code>lst</code> query example:
<pre>
$ cwigSummary lst data.cwig chr1 300000 330000
chr1	258729	320867	0
chr1	320867	320872	0.00079
chr1	320872	320877	0.00089
chr1	320877	320883	0.001
             ...
chr1	324138	324139	0.00089
chr1	324139	333869	0
</pre>

<code>map</code> query example:
<pre>
$ cwigSummary map data.cwig chr1 324125 324135
0.0012 0.0012 0.0012 0.0012 0.00109 0.00109 0.00109 0.00109 0.001 0.001
</pre>

<h3>Batch query cwig file</h3>
<code>cwigSummaryBatch</code> is similar to <code>cwigSummary</code> program. However, the input regions are read from a query file (one region per line).
The syntax is:
<pre>
# cwigSummaryBatch [avg|cov|min|max|lst|map] (cwig_file) (query_file) [window_count=1]
</pre>

For example, the query_file may look like:
<pre>
chr1 100 200
chr2 200 300
chrX 300 400
</pre>

It is recommended to use <code>cwigSummaryBatch</code> rather than call <code>cwigSummary</code> multiple times. Especially when the cwig file is not in the same computer to avoid initial loading time.

<h3>Print basic information of cwig file</h3>
This command load and check the integrity of a cwig file. It also prints basic information.
<pre>
# cwigInfo (cwig_file)
</pre>



<h2>API</h2>

All the functions of the commandline tools can be used directly through the API. We provides native C++ API. Python and Java APIs are available through SWIG wrappers.
To use the the API, include the cwig sub-folder, and uses the classes in <code>cwig.h</code> and <code>chrfmt.h</code> headers.
The detailed class documentations will be available soon.
<a id="exp">
<h1>Experiments</h1>
</a>
The dataset used in the paper can be download at <a href="http://genome.ucsc.edu/ENCODE/downloads.html">UCSC Encode database</a>. The bigWig files can be convert to cwig files using the convertion tools.
To measure the query time of cwig, please use the <code>cwigSummaryBatch</code> tool. For query bigWig file in batch, you can use our modification of <a href="./files/bigWigSummaryBatch.tar.gz"><code>bigWigSummaryBatch</code></a>.

<a id="info">
<h1>Information</h1>
</a>

<h2>License</h2>
CWig is released under the LGPL License.<br>
Copyright (c) 2012-2014 Do Huy Hoang and Sung Wing-Kin
<br/>

<h2>Citation</h2>
Do.H.H and Sung.W.K. CWig: Compressed representation of Wiggle/BedGraph format. Bioinformatics. 2014
<h2>Acknowledgements</h2>
&nbsp;
<h2>Contacts</h2>
<a href="mailto:dohh@gis.a-star.edu.sg">Hoang</a> or <a href="mailto:ksung@comp.nus.edu.sg">ksung</a>