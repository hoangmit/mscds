/*
 *
 *  $Id: cache_set.hpp 18 2007-01-21 15:49:02Z matteo.merli $
 *
 *  $URL: https://cache-table.googlecode.com/svn/tags/0.2/mm/cache_set.hpp $
 *
 *  Copyright (C) 2006 Matteo Merli <matteo.merli@gmail.com>
 *
 *
 *  BSD License
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *   o Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   o Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the
 *     distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _CACHE_SET_HPP_
#define _CACHE_SET_HPP_

#include "cache_table.hpp"
#include "hash_fun.hpp"

namespace mm
{

/** Default "discarding" policy.
 *  Ignores discarded item. You can write another policy to do something
 *  useful with this item.
 */
template < class V >
struct CacheSetDiscardIgnore
{
    /** This operator is called when the map need to discard an
     * item due to a key collision.
     * 
     * @param old_value a reference to the element discarded
     * @param new_value a reference to the element that will enter 
     *                  in the map
     * 
     */
    void operator() ( const V& old_value, const V& new_value )
    {}    
};

/** Class used as the @a KeyExtract template parameter for CacheTable.
 * 
 * In the case of CacheSet, since the @a key correspond with the 
 * entire stored object, the function must be the @a Identity.
 * 
 */
template <class Value>
struct Identity
{

    /** This operator is called to extract the @a key of an item
     * 
     * @param value the item 
     * @return the key of the item which correspond to the item itself
     */
    Value& operator() ( Value& value ) const { return value; }
    
    /** This operator is called to extract the @a key of an item
     * 
     * @param value the item 
     * @return the key of the item which correspond to the item itself
     */
    const Value& operator() ( const Value& value ) const { return value; }
};


/** CacheSet class. 
 * 
 * Implements a @a set container like std::hash_set, but with a 
 * fixed element number. 
 * 
 *  <b>Template Parameters</b>
 * 
 *  - @a Value : The type of items contained in the set.   
 *  - @a HashFunction : Callable hasher.
 *  - @a DiscardFunction : Discarded item manager.
 *  - @a Allocator : Allocator to be used.
 *
 *  @author Matteo Merli
 *  @date $Date: 2007-01-21 16:49:02 +0100 (Sun, 21 Jan 2007) $
 */
template < class Value,
           class HashFunction = hash<Value>,
           class KeyEqual = std::equal_to<Value>,
           class DiscardFunction = CacheSetDiscardIgnore< Value >,
           class Allocator = std::allocator<Value> // not used
>
class cache_set
{
private:
    /// The actual hash table
    typedef cache_table< Value, Value,
                         DiscardFunction,
                         HashFunction, KeyEqual,
                         Identity<Value>,
                         Allocator
                       > HT;
    HT m_ht;
        
public:

    /// The cache_set's key type, Key. 
    typedef typename HT::key_type key_type;

    /// The object type
    typedef Value data_type;
    
    /// The object type
    typedef Value mapped_type;

    /// The type of object, pair<const key_type, data_type>, stored in the
    /// cache_set.
    typedef typename HT::value_type value_type;

    /// The cache_set's hash function. 
    typedef typename HT::hasher hasher;

    /// Function object that compares keys for equality. 
    typedef typename HT::key_equal key_equal;

    /// An unsigned integral type. 
    typedef typename HT::size_type size_type;

    /// A signed integral type. 
    typedef typename HT::difference_type difference_type;

    /// Pointer to value_type.
    typedef typename HT::const_pointer pointer;

    /// Const pointer to value_type. 
    typedef typename HT::const_pointer const_pointer;

    /// Reference to value_type.
    typedef typename HT::const_reference reference;

    /// Const reference to value_type.
    typedef typename HT::const_reference const_reference;

    /// Iterator used to iterate through a cache_set. 
    typedef typename HT::const_iterator iterator;
    
    /// Const iterator used to iterate through a cache_set. 
    typedef typename HT::const_iterator const_iterator;
    
    
public:
    /** Returns the hasher object used by the cache_set. 
     */ 
    hasher hash_funct() const { return m_ht.hash_funct(); }

    /// Returns the key_equal object used by the cache_set. 
    key_equal key_eq() const  { return m_ht.key_eq(); }

    /// Get an iterator to first item
    iterator begin()             { return m_ht.begin(); }
    /// Get an iterator to the end of the table
    iterator end()               { return m_ht.end();   }
    /// Get a const iterator to first item
    const_iterator begin() const { return m_ht.begin(); }
    /// Get a const iterator to the end of the table
    const_iterator end()   const { return m_ht.end();   }
    
public:
    
    /** Constructor. Construct an empty set with the default
     * number of buckets.
     * 
     */
    cache_set() : m_ht() {}

    /** Constructor.  You have to specify the size of the underlying table,
     *  in terms of the maximum number of allowed elements.
     * 
     *  @attention After the cache_set is constructed, you have to call the
     *  set_empty_key() method to set the value of an unused key.
     *
     *  @param n The (fixed) size of table.
     *  @see set_empty_key
     */
    cache_set( size_type n ) : m_ht( n, hasher(), key_equal() ) {}

    /** Constructor. 
     * 
     * @param n the number of item buckets to allocate.
     * @param hash the hasher function 
     */
    cache_set( size_type n, const hasher& hash )
        : m_ht( n, hash, key_equal() )
    {}

    /** Constructor. 
     * 
     * @param n    the number of item buckets to allocate
     * @param hash the hasher function
     * @param ke   the key comparison function
     */
    cache_set( size_type n,
               const hasher& hash,
               const key_equal& ke  )
        : m_ht( n, hash, ke )
    {}

    /** Constructor. Creates a cache_set with a copy of a range.
     * 
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     */
    template <class InputIterator>
    cache_set( InputIterator first, InputIterator last )
        : m_ht(  2 * std::distance( first, last ) )
    {
        m_ht.insert( first, last );
    }

    /** Constructor. Creates a cache_set with a copy of a range and a 
     *  bucket count of at least @a n.
     * 
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     */
    template <class InputIterator>
    cache_set( InputIterator first, InputIterator last, size_type n )
            : m_ht(  2 * n )
    {
        m_ht.insert( first, last );
    }

    /** Creates a cache_set with a copy of a range and a bucket count of at
     *  least @a n, using @a h as the hash function.
     * 
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     * @param h     hasher function to be used
     */
    template <class InputIterator>
    cache_set( InputIterator first, InputIterator last,
              size_type n, const hasher& h )
        : m_ht(  2 * n, h )
    {
        m_ht.insert( first, last );
    }

    /** Creates a cache_set with a copy of a range and a bucket count of at
     *  least n, using h as the hash function and k as the key equal
     *  function.
     * 
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     * @param h     hasher function to be used
     * @param k     key comparison function to be used
     */
    template <class InputIterator>
    cache_set( InputIterator first, InputIterator last,
              size_type n, const hasher& h, 
              const key_equal& k )
        : m_ht(  2 * n, h, k )
    {
        m_ht.insert( first, last );
    }

    /** Copy constructor
     *
     *  @param other the cache_set to be copied
     */
    cache_set( const cache_set& other )
        : m_ht( other.m_ht )
    {}

    /** The assignment operator
     *
     *  @param other the cache_set to be copied
     *  @return a reference to a new cache_set object
     */
    cache_set& operator= ( const cache_set& other )
    {
        if ( &other != this )
        {
            m_ht = other.m_ht;
        }
        
        return *this;
    }

    /** Sets the value of the empty key.
     *   
     *  @param value the value value that will be used to identify empty items.
     */
    void set_empty_key( const value_type& value )
    {
        m_ht.set_empty_value( value );
    }
    
    /** Get the value of the empty item.
     * 
     * @return the item value used to mark empty buckets. 
     */
    const value_type& get_empty_key() const
    {
        return m_ht.get_empty_value();
    }

    /** Insert an item in the set.
     *
     *  The item, the pair(key, data), will be inserted in the hash table.
     *  In case of a key hash collision, the inserted item will replace the
     *  existing one.
     *
     *  Following the @a DiscardFunction policy there will be a notification
     *  that old item has been replaced.
     * 
     *  @return A @p pair<iterator,bool> which contain an #iterator to the
     *  inserted item and @p true if the item was correctly inserted, or @p
     *  false if the item was not inserted.
     */
    pair<iterator,bool> insert( const value_type& obj )
    { return m_ht.insert( obj ); }

    /** Iterator insertion.
     *  Insert multiple items into the set, using the input iterators.
     *
     *  @param first first elelement 
     *  @param last last element
     *  @see insert( const value_type& )
     */
    template <class InputIterator>
    void insert( InputIterator first, InputIterator last )
    { m_ht.insert( first, last ); }

    /** Not standard iterator insertion
     * 
     * @param it iterator that mark the insertion position
     * @param obj the new value for the item
     * @return an iterator to the modified item
     */
    iterator insert( iterator it, const value_type& obj )
    { return m_ht.insert( obj ).first; }

    /** Finds an element in the set.
     *
     *  @param item the item to look for
     *
     *  @return a #const_iterator pointing to the item, or @p end() if the
     *  item cannot be found in the set.
     */
    iterator find( const value_type& item ) const 
    { return m_ht.find( item ); }

    /** Erases the element identified by the key. 
     *
     *  @param key The key of the item to be deleted.
     *
     *  @return The number of deleted item. Since the item in cache_set are
     *  unique, this will either be 1 when the key is found and 0 when no
     *  item is deleted.
     */
    size_type erase( const key_type& key ) { return m_ht.erase( key ); }
    
    /** Erases the element pointed to by the iterator. 
     *
     *  @param it a valid iterator to an element in cache_set.
     */
    void erase( iterator it ) { m_ht.erase( it ); }

    /** Erases all elements in a range.
     *
     *  The range of elements to be deleted will be @p [first,last)
     *
     *  @param first The first element in the range (included)
     *  @param last  The last element in the range (excluded)
     */
    void erase( iterator first, iterator last ) { m_ht.erase( first, last ); }

    /** Removes all the elements. */
    void clear() { m_ht.clear(); }

    /** Increases the bucket count to at least @a size.
     *
     *  @param size the new maximum number of elements.
     *
     *  @warning This operation can be particularly time-consuming. The
     *  algorithm is O(n) and can leave to the complete re-hash of all
     *  of the elements in the cache_set.
     */
    void resize( size_type size ) { m_ht.resize( size ); }

    /** Swap the content of two cache_set.
     *
     *  @param other another cache_set
     */
    void swap( cache_set& other ) { m_ht.swap( other.m_ht ); }

    /** Equality comparison.
     *
     *  @param other The other cache_set to compare.
     *  @return @p true if the 2 cache_set are equal.
     */
    bool operator==( const cache_set& other ) const
    { return m_ht == other.m_ht; }

    /** Dis-Equality comparison.
     *
     *  @param other The other cache_set to compare.
     *  @return @p true if the 2 cache_set are @a NOT equal.
     */
    bool operator!=( const cache_set& other ) const
    { return m_ht != other.m_ht; }

    /** Get the size of the cache_set.
     *  The size represent the number of non-empty elements that can be
     *  found in the container.
     *  @return the number of elements in the set
     */
    size_type size()         const { return m_ht.size(); }
    
    /** Get the maximum size of the set.
     *  This corresponds to the maximum number of item that the set can
     *  contain.
     *  @return the maximum number of elements
     */
    size_type max_size()     const { return m_ht.max_size(); }

    /** Get the number of allocated buckets.
     *  This corresponds to the max_size() value.
     *  @return the number of buckets
     *  @see max_size
     */
    size_type bucket_count() const { return m_ht.bucket_count(); }
    
    /** Test for empty.
     *  @return true if the cache_set does not contains items.
     */
    bool empty()             const { return m_ht.empty(); }

    /** Get the number of hash key collisions.
     * 
     *  @return the number of hash key collisions
     */
    size_type num_collisions() const { return m_ht.num_collisions(); }

    /** Swap the content of two cache_set instances.
     *
     *  @param m1 a cache_set
     *  @param m2 another cache_set
     */
    friend inline void swap(
        cache_set<Value,HashFunction,KeyEqual,DiscardFunction,Allocator>& m1,
        cache_set<Value,HashFunction,KeyEqual,DiscardFunction,Allocator>& m2 )
    {
        m1.swap( m2 );
    }
    
};
    
} // namespace mm

#endif // _CACHE_SET_HPP_
