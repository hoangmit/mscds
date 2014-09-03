namespace mscds {
class BitArrayInterface {
public:
	const static unsigned int WORDLEN = 64;
	
	/* read one by at position `bitindex` */
	bool bit(size_t bitindex) const;
	
	/* read `len` bits at at position `bitindex`. The maximal number of bits can be read is 64  */
	uint64_t bits(size_t bitindex, unsigned int len) const;
	/* same as bit() function */
	bool operator[](size_t i) const;
	
	/* change a bit at postion `bitindex` */
	void setbit(size_t bitindex, bool value);
	
	/* take `len` bits in variable `value` and put the the bitarray */
	void setbits(size_t bitindex, uint64_t value, unsigned int len);	
	
	/* read 8 bits at position `pos` * 8 */
	uint8_t byte(size_t pos) const;
	
	/* return the number of one bits in the whole array */
	uint64_t count_one() const;
	
	/* fill the array with zeros (false) */
	void fillzero();
	
	/* fill the array with ones (true) */
	void fillone();

	/* read 64 bits at position `pos`*64 */
	uint64_t& word(size_t pos);
	const uint64_t& word(size_t pos) const;
	
	/* return the number of bits in the array */
	size_t length();
	
	/* return the number of words */
	size_t word_count()

//--------------------------------------------------
	/* constructor: empty array */
	BitArray() {}

	/* constructor: create an bitarray with `bit_len` bits e.g. BitArray a(100); */
	BitArray(size_t bit_len);	
	
	/* constructor: from other array */
	BitArray(const BitArray& other);
	
	/* static function to create e.g. BitArray a = BitArray::create(100); */
	static BitArray create(size_t bitlen);

	BitArray& operator=(const BitArray& other);	
	BitArray(SharedPtr p, size_t bit_len);

	BitArray(uint64_t * ptr, size_t bit_len);

	/* clear the array (size becomes 0) */
	void clear();

	/* return array with data */
	const uint64_t* data_ptr() const;


	/* static function to copy from array of words */
	static BitArray create(uint64_t * ptr, size_t bitlen);

	BitArray clone_mem() const;


	/* save/load functions */
	IArchive& load(IArchive& ar);
	OArchive& save(OArchive& ar) const;
	OArchive& save_nocls(OArchive& ar) const;
	IArchive& load_nocls(IArchive& ar);
	
	/* return the string for debug */
	std::string to_str() const;
};
}//namespace
