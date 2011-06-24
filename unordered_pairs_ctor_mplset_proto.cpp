//Purpose:
//  Prototype of constructor taking arguments
//  in order different than they were declared
//  in the composite template arguments.
//Motivation:
//  Post with headers:
/*
From: "Dean Michael C. Berris" <dmberris@friendster.com>
Newsgroups: gmane.comp.parsers.spirit.devel
Subject: [fusion] Constructing fusion::map<P1, P2, ..,
	PN> Requires same ordering of P's?
Date: Wed, 31 Oct 2007 08:02:16 -0700

 */
#include <boost/fusion/support/pair.hpp> 
#include <boost/fusion/sequence/intrinsic/empty.hpp> 
#include <boost/fusion/sequence/intrinsic/begin.hpp> 
#include <boost/fusion/sequence/intrinsic/end.hpp> 
#include <boost/fusion/container/vector.hpp> 
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/view/iterator_range.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/mpl/erase_key.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <iostream>

namespace boost
{
namespace fusion
{
namespace result_of
{
    template
    < class First
    , class Last
    >
    struct
  deref
    < iterator_range<First,Last>
    >
  {
          typedef
          typename
        remove_reference
        < typename deref<First>::type
        >::type
      type
      ;
  };
    template
    < class First
    , class Last
    >
    struct
  next
    < iterator_range<First,Last>
    >
  {
          typedef
        iterator_range
        < typename next<First>::type
        , Last
        >
      type
      ;
  };
}//exit result_of namespace
    template
    < class First
    , class Last
    >
    typename result_of::deref
    < iterator_range
      < First
      , Last
      >
    >::type const&
  deref
    ( iterator_range<First,Last>const &a_range
    )
  {
      return deref(a_range.first);
  }
    template
    < class First
    , class Last
    >
    typename result_of::next
    < iterator_range
      < First
      , Last
      >
    >::type
  next
    ( iterator_range<First,Last>const &a_range
    )
  {
      typedef typename result_of::next<First>::type first_type;
      return iterator_range<first_type,Last>(next(a_range.first),a_range.last);
  }
}//exit fusion namespace
}//exit boost namespace

using namespace boost;

    static
  unsigned
instances
=0
;
  template
  < unsigned TypeId
  >
  struct
type_id
{
      static unsigned const
    our_type_id=TypeId
    ;
      unsigned const
    my_instance_id
    ;
    type_id(void)
    : my_instance_id(++instances)
    {}
    type_id(type_id const&)
    : my_instance_id(++instances)
    {}
      void
    operator=(type_id const&)
    {}
      unsigned
    get_instance_id(void)const
    {
          return my_instance_id;
    }
      unsigned
    get_type_id(void)const
    {
          return our_type_id;
    }
};
  template
  < class MapAll
  >
  struct
composite_storage_inserter
{
        static
      unsigned const
    our_size
    = 1 //in real code, this would be the size of the composite.
    ;
      char
    storage //memory for composite
      [ our_size
      ]
    ;
      template
      < class KeyVal
      >
      void
    insert_val_at_key
      ( KeyVal const& a_key_val
      )
    {
        //real code would insert a_key_val.second in part
        //of storage corresponding to KeyVal::first_type.
        typedef typename KeyVal::first_type  key_type;
        BOOST_MPL_ASSERT((mpl::has_key<MapAll,key_type>));
        typedef typename KeyVal::second_type pair_val_type;
        typedef typename mpl::at<MapAll,key_type>::type map_val_type;
        BOOST_MPL_ASSERT((is_same<pair_val_type,map_val_type>));
        std::cout
          <<"insert val{ type_id=>"
          <<a_key_val.second.get_type_id()
          <<", instance_id=>"
          <<a_key_val.second.get_instance_id()
          <<"}\n";
        std::cout
          <<"at key val{ type_id=>"
          <<key_type::our_type_id
          <<"}\n";
    }
};

  template
  < class MapAll
  >
  struct
composite_one_def_ctor
{
      template
      < class KeyVal
      >
      void
    operator()
      ( KeyVal
      )
    //inserts a_key_val.first into my_storage_inserter.
    {
        std::cout<<"{construct_def_one\n";
        typedef fusion::pair<typename KeyVal::first,typename KeyVal::second> pair_type;
        pair_type a_pair;
        my_storage_inserter.insert_val_at_key(a_pair);
        std::cout<<"}construct_def_one\n";
    }
    composite_one_def_ctor
      ( composite_storage_inserter<MapAll>& a_storage_inserter
      )
    : my_storage_inserter(a_storage_inserter)
    {}
      composite_storage_inserter<MapAll>&
    my_storage_inserter
    ;
};
  template
  < class MapAll  //Map for complete composite
  , class MapArgs //Map for part of composite with unconstructed components.
  >
  struct
composite_one_ctor
{
        typedef
      composite_storage_inserter<MapAll>
    posite_type  
    ;  
      template
      < class KeyValIter
      >
          static
        typename
      enable_if
      < typename mpl::not_<typename fusion::result_of::empty<KeyValIter>::type>::type
      , void
      >::type
    construct_one
      ( posite_type& a_posite
      , KeyValIter const& a_key_val_iter
      )
    //inserts deref of a_key_val_iter into a_posite
    //and typedefs an augmented ArgKeys by inserted Key of deref,
    //then creates another instance of this tmeplate with the 
    //augmented ArgKeys and recurses with next of a_key_val_iter. 
    {
        std::cout<<"{construct_arg_one:size="<<fusion::size(a_key_val_iter)<<"\n";
        typedef typename fusion::result_of::deref<KeyValIter>::type key_val_pair_type;
        typedef typename key_val_pair_type::first_type key_type;
        typedef typename mpl::has_key<MapArgs,key_type>::type key_found;
        BOOST_MPL_ASSERT((key_found));
        key_val_pair_type const&a_key_val=fusion::deref(a_key_val_iter);
        a_posite.insert_val_at_key(a_key_val);
        typedef typename mpl::erase_key<MapArgs,key_type>::type map_args_type;
        typedef composite_one_ctor<MapAll,map_args_type> next_coc_type;
        typedef typename fusion::result_of::next<KeyValIter>::type next_iter_type;
        next_iter_type const next_iter_valu(fusion::next(a_key_val_iter));
        next_coc_type::template construct_one<next_iter_type>(a_posite,next_iter_valu);
        std::cout<<"}construct_arg_one:size="<<fusion::size(a_key_val_iter)<<"\n";
    }
      template
      < class KeyValIter
      >
        static
        typename
      enable_if
      < typename fusion::result_of::empty<KeyValIter>::type
      , void
      >::type
    construct_one
      ( posite_type& a_posite
      , KeyValIter const&
      )
    //For all the Keys not in ArgKeys, inserts a default Vals
    //into a_posite.
    {
        composite_one_def_ctor<MapAll> def_ctor(a_posite);
        mpl::for_each<MapArgs>(def_ctor);
    }
};
  template
  < class MapAll
  >
  struct
composite_ctor
: composite_storage_inserter<MapAll>
{
      template
      < class PairSeq
      >
    composite_ctor
      ( PairSeq a_pairs
      )
    {
        typedef composite_one_ctor<MapAll,MapAll> coc_type;
        typedef typename fusion::result_of::begin<PairSeq>::type beg_type;
        typedef typename fusion::result_of::end<PairSeq>::type end_type;
        typedef fusion::iterator_range<beg_type,end_type> iter_type;
        iter_type pair_iter(fusion::begin(a_pairs),fusion::end(a_pairs));
        std::cout<<"{construct_one:size="<<fusion::size(pair_iter)<<"\n";
        coc_type::construct_one(*this,pair_iter);
        std::cout<<"}construct_one:size="<<fusion::size(pair_iter)<<"\n";
    }
      unsigned
    return0(void)const
    {
        return 0;
    }
};
#include <boost/fusion/container/vector.hpp>

int main(void)
{
        typedef 
      mpl::map
      < mpl::pair<type_id<1>,type_id<1> >
      , mpl::pair<type_id<2>,type_id<2> >
      , mpl::pair<type_id<3>,type_id<3> >
      >::type
    map_type;
        typedef
      fusion::vector
      < fusion::pair<type_id<3>,type_id<3> >
      , fusion::pair<type_id<1>,type_id<1> >
      >
    pairs_type;
    pairs_type a_pairs;
    std::cout<<"{create composite\n";
    composite_ctor<map_type> a_posite(a_pairs);
    std::cout<<"}create composite\n";
    return a_posite.return0();
}    
