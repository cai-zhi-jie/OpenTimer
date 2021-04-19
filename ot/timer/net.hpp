#ifndef OT_TIMER_NET_HPP_
#define OT_TIMER_NET_HPP_

#include <ot/spef/spef.hpp>
#include <ot/timer/pin.hpp>
#include <ot/traits.hpp>

namespace ot {

// Forward declaration
class RctEdge;
class RctNode;
class Rct;

// ------------------------------------------------------------------------------------------------

// Class: RctNode
class RctNode {

  friend class Rct;
  friend class Net;
  friend class Timer;
  
  public:

    RctNode() = default;
    RctNode(const std::string&);

    float load (Split, Tran) const;
    float cap  (Split, Tran) const;
    float slew (Split, Tran, float) const;
    float delay(Split, Tran) const;

  private:

    std::string _name;                           

    TimingData<float, MAX_TRAN, MAX_SPLIT> _ures   ; 
    TimingData<float, MAX_TRAN, MAX_SPLIT> _ncap   ;
    TimingData<float, MAX_TRAN, MAX_SPLIT> _load   ; 
    TimingData<float, MAX_TRAN, MAX_SPLIT> _beta   ;
    TimingData<float, MAX_TRAN, MAX_SPLIT> _delay  ;
    TimingData<float, MAX_TRAN, MAX_SPLIT> _ldelay ;
    TimingData<float, MAX_TRAN, MAX_SPLIT> _impulse;

    std::list<RctEdge*> _fanin;
    std::list<RctEdge*> _fanout;

    Pin* _pin {nullptr};

    void _scale_capacitance(float);
};

// ------------------------------------------------------------------------------------------------

// Class: RctEdge
class RctEdge {

  friend class Rct;
  friend class Net;
  friend class Timer;
  
  public:
    
    RctEdge(RctNode&, RctNode&, float);

    inline float res() const;
    inline void res(float);
  
  private:

    RctNode& _from;
    RctNode& _to;
    
    float _res {0.0f};

    void _scale_resistance(float);
};

// Function: res
inline float RctEdge::res() const {
  return _res;
}

// Procedure: res
inline void RctEdge::res(float v) {
  _res = v; 
}

// ------------------------------------------------------------------------------------------------

// Class: Rct
class Rct {

  friend class Net;
  friend class Timer;

  public:

    void update_rc_timing();
    void insert_segment(const std::string&, const std::string&, float);
    void insert_node(const std::string&, float = 0.0f);
    void insert_edge(const std::string&, const std::string&, float);
    
    float total_ncap() const;
    float slew(const std::string&, Split, Tran, float) const;
    float delay(const std::string&, Split, Tran) const;

    inline size_t num_nodes() const;
    inline size_t num_edges() const;
    
    const RctNode* node(const std::string&) const;

  private:

    RctNode* _root {nullptr};

    std::unordered_map<std::string, RctNode> _nodes;
    std::list<RctEdge> _edges;

    void _update_load(RctNode*, RctNode*);
    void _update_delay(RctNode*, RctNode*);
    void _update_ldelay(RctNode*, RctNode*);
    void _update_response(RctNode*, RctNode*);
    void _scale_capacitance(float);
    void _scale_resistance(float);

    RctNode* _node(const std::string&);
};

// Function: num_nodes
inline size_t Rct::num_nodes() const {
  return _nodes.size();
}

// Function: num_edges
inline size_t Rct::num_edges() const {
  return _edges.size();
}

// ------------------------------------------------------------------------------------------------

// Class: Net
class Net {

  friend class Timer;
  friend class Arc;
  friend class Pin;
  
  struct EmptyRct {
    // This struct wraps a two-dimensional array with a specific size
    // (MAX_SPLIT (rows), MAX_TRAN (columns))
    std::array<std::array<float, MAX_TRAN>, MAX_SPLIT> load;
  };

  public:
    // Net class contains a default constructor, but Pin class does not.
    // The operator[] is not available for map with Net as value type.
    Net() = default;
    Net(const std::string&);

    inline const std::string& name() const { return _name; }
    inline size_t num_pins() const { return _pins.size(); }

    inline const Rct* rct() const { return std::get_if<Rct>(&_rct); }
    inline Rct* rct() { return std::get_if<Rct>(&_rct); }

    // The RCTree instance variable is actually a variant. The following
    // setter is invalid because the copy constructor is implicitly
    // deleted because the default definition would be ill-formed.
    //
    //    inline Net* rct(Rct& tree) { _rct = tree; return this; }
    // -------

    // Non-const accessors/mutators may be dangerous.
    inline const Pin* root() const { return _root; }
    inline Pin* root() { return _root; } // mutable accessor.
    inline Net* root(Pin& rt) { _root = &rt; return this; } // setter/mutator

    // Insert a new pin. Usually this kind of operations are not visible
    // to the users as it should be done in parsers.
    // When emulating the parser behaviours, they will be useful.
    inline Net* append(Pin& pin) { _pins.emplace_back(&pin); return this; }

  private:

    std::string _name;

    Pin* _root {nullptr};

    std::list<Pin*> _pins;

    std::variant<EmptyRct, Rct> _rct;

    std::optional<spef::Net> _spef_net;

    bool _rc_timing_updated {false};

    float _load(Split, Tran) const;

    std::optional<float> _slew(Split, Tran, float, Pin&) const;
    std::optional<float> _delay(Split, Tran, Pin&) const;
    
    void _update_rc_timing();
    void _attach(spef::Net&&);
    void _make_rct();
    //void _make_rct(const spef::Net&);
    void _insert_pin(Pin&);
    void _remove_pin(Pin&);
    void _scale_capacitance(float);
    void _scale_resistance(float);
}; 

};  // end of namespace ot. -----------------------------------------------------------------------

#endif






