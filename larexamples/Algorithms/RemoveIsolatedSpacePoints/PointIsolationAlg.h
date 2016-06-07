/**
 * @file   PointIsolationAlg.h
 * @brief  Algorithm(s) dealing with point isolation in space
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   May 27, 2016
 * 
 * This library contains only template classes and it is header only.
 * 
 */

#ifndef LAREXAMPLES_ALGORITHMS_REMOVEISOLATEDSPACEPOINTS_POINTISOLATIONALG_H
#define LAREXAMPLES_ALGORITHMS_REMOVEISOLATEDSPACEPOINTS_POINTISOLATIONALG_H


// infrastructure and utilities
#include "cetlib/pow.h" // cet::sum_squares()

// C/C++ standard libraries
#include <vector>
#include <array>
#include <string>
#include <iterator> // std::cbegin(), std::cend()
#include <stdexcept> // std::runtime_error()

#if ((__GNUC__ == 5) && (__GNUC_MINOR__ >= 1)) || (__GNUC__ >= 6)
// FIXME GCC51
// when this error will be triggered, fix the code to use std::cbegin() and
// std::cend() and remove this check
# error "Update to use std::cbegin()/std::cend() with GCC 5.1 and above"
#endif


namespace lar {
  namespace example {
    
    /**
     * @brief  Helper extractor for point coordinates
     * @tparam Point type of point structure
     * 
     * The mandatory interface is:
     * 
     * - `static T x(Point const& point)`: return x coordinate of point
     * - `static T y(Point const& point)`: return y coordinate of point
     * - `static T z(Point const& point)`: return z coordinate of point
     * 
     * The type T must be convertible to a number (typically a real one).
     * Examples of specialisation:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *     struct SpaceTime_t {
     *       double x, y, z; ///< space coordinates
     *       double t;       ///< time coordinate
     *     };
     *     
     *     namespace lar {
     *       namespace examples {
     *         
     *         template <>
     *         struct PositionExtractor<SpaceTime_t> {
     *           
     *           static double x(SpaceTime_t const& st) { return st.x; }
     *           static double y(SpaceTime_t const& st) { return st.y; }
     *           static double z(SpaceTime_t const& st) { return st.z; }
     *           
     *         };
     *         
     *         template <>
     *         struct PositionExtractor<SpaceTime_t const*> {
     *           
     *           static double x(SpaceTime_t const* st) { return st->x; }
     *           static double y(SpaceTime_t const* st) { return st->y; }
     *           static double z(SpaceTime_t const* st) { return st->z; }
     *           
     *         };
     *         
     *       }
     *     }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * The argument of the function just needs to be something the template
     * argument can be converted into: in the second example, the argument of
     * `X` is not required to br exactly `SpaceTime_t const* const&`.
     */
    template <typename Point>
    struct PositionExtractor;
    
    
    /**
     * @brief Algorithm to detect isolated space points
     * @tparam Coord type of the coordinate
     * 
     * This algorithm returns a selection of the input points which are not
     * isolated. Point @f$ i @f$ is defined as isolated if:
     * @f$ \min \left\{ \left| \vec{r}_{i} - \vec{r_{j}} \right| \right\}_{i \neq j} < R @f$
     * where @f$ \vec{r_{k}} @f$ describes the position of the point @f$ k @f$
     * in space and @f$ R @f$ is the isolation radius.
     * 
     * This class must be configured by providing a complete Configuration_t
     * object. Configuration can be changed at any time after that.
     * 
     * The configuration information (`Configuration_t`) defines the volume the
     * points span and the square of the isolation radius.
     * The information on the volume may be used to optimise the algorithm,
     * and it is not checked. If that information is wrong (that means input
     * points lie outside that volume), the result is undefined.
     * No check is automatically performed to assess if the configuration is
     * valid.
     * 
     * The algorithm can be run on any collection of points, as long as the
     * point class supports the `PositionExtractor` class.
     * A typical cycle of use is:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *     // creation and configuration
     *     lar::examples::PointIsolationAlg<float>::Configuration_t config;
     *     config.rangeX = { -1., 1. };
     *     config.rangeY = { -1., 1. };
     *     config.rangeZ = { -5., 5. };
     *     config.radius2 = 0.25;
     *     
     *     lar::examples::PointIsolationAlg<float> algo(config);
     *     
     *     // preparation/retrieval of input
     *     std::vector<std::array<float, 3>> points;
     *     // points are filled here
     *     
     *     // execution
     *     std::vector<size_t> indices
     *       = algo.removeIsolatedPoints(points.begin(), points.end());
     *     
     *     // utilization of the result;
     *     // - e.g., create a collection of non-isolated points...
     *     std::vector<std::array<float, 3>> nonIsolatedPoints;
     *     nonIsolatedPoints.reserve(indices.size());
     *     for (size_t index: indices)
     *       nonIsolatedPoints.push_back(points[index]);
     *     
     *     // - ... or their pointers
     *     std::vector<std::array<float, 3> const*> nonIsolatedPointPtrs;
     *     nonIsolatedPointPtrs.reserve(indices.size());
     *     for (size_t index: indices)
     *       nonIsolatedPointPtrs.push_back(&(points[index]));
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * The point type here is `std::array<float, 3>`, for which a
     * `lar::examples::PositionExtractor<std::array<float, 3>>` is defined
     * in this same library.
     * The algorithm can be executed multiple times, and the configuration can
     * be changed at any time (`reconfigure()`).
     * 
     * Validation of the configuration is optional, and needs to be explicitly
     * called if desired (`validateConfiguration()`).
     */
    template <typename Coord = double>
    class PointIsolationAlg {
      
        public:
      /// Type of coordinate
      using Coord_t = Coord;
      
      /// Range of coordinates
      struct Range_t {
        Coord_t lower; ///< lower boundary
        Coord_t upper; ///< upper boundary
        
        /// Returns whether c is contained in the range (inclusve)
        bool contains(Coord_t c) const;
        
        /// Returns whether the range is empty
        bool empty() const;
        
        /// Returns whether the range is valid (empty is also valid)
        bool valid() const;
        
        /// Returns the size of the range (no check)
        Coord_t size() const;
        
        /// Returns whether the specified range has the same limits as this
        bool operator== (const Range_t& as) const;
        
        /// Returns whether the specified range has limits different than this
        bool operator!= (const Range_t& than) const;
        
      }; // Range_t
      
      /// Type containing all configuration parameters of the algorithm
      struct Configuration_t {
        Range_t rangeX;   ///< range in X of the covered volume
        Range_t rangeY;   ///< range in Y of the covered volume
        Range_t rangeZ;   ///< range in Z of the covered volume
        Coord_t radius2;  ///< square of isolation radius [cm^2]
      }; // Configuration_t
      
      
      /// @{
      /// @name Configuration
      
      /**
       * @brief Constructor with configuration validation
       * @param first_config configuration parameter structure
       * 
       * For the configuration, see `SpacePointIsolationAlg` documentation.
       * No validation is performed on the configuration.
       */
      PointIsolationAlg(Configuration_t const& first_config)
        : config(first_config)
        {}
      
      /// Reconfigures the algorithm with the specified configuration
      /// (no validation is performed)
      /// @see configuration()
      void reconfigure(Configuration_t const& new_config)
        { config = new_config; }
      
      
      /// Returns a constant reference to the current configuration
      /// @see reconfigure()
      Configuration_t& configuration() const { return config; }
      
      /// @}
      
      
      /**
       * @brief Returns the set of points that are not isolated
       * @tparam PointIter random access iterator to a point type
       * @param begin iterator to the first point to be considered
       * @param end iterator after the last point to be considered
       * @return a list of indices of non-isolated points in the input range
       * 
       * This method is the operating core of the algorithm.
       * 
       * The input is two iterators. The output is a collection of the
       * indices of the elements that are not isolated. The index is equivalent
       * to `std::distance(begin, point)`.
       * The order of the elements in the collection is not specified.
       * 
       * This method can use any collection of input data, as long as a
       * `PositionExtractor` object is available for it.
       */
      template <typename PointIter>
      std::vector<size_t> removeIsolatedPoints
        (PointIter begin, PointIter end) const;
      
      
      /**
       * @brief Returns the set of points that are not isolated
       * @param points list of the reconstructed space points
       * @return a list of indices of non-isolated points in the vector
       * @see removeIsolatedPoints(PointIter begin, PointIter end) const
       */
      template <typename Cont>
      std::vector<size_t> removeIsolatedPoints (Cont const& points) const
        { return removeIsolatedPoints(std::begin(points), std::end(points)); }
// FIXME GCC51
//        { return removeIsolatedPoints(std::cbegin(points), std::cend(points)); }
      
      
      /// @{
      /// @name Configuration
      
      /// Throws an exception if the configuration is invalid
      /// @throw std::runtime_error if configuration is invalid
      static void validateConfiguration(Configuration_t const& config);
      
      /// @}
      
        private:
      
      Configuration_t config; ///< all configuration data
      
      /// Returns whether A and B are close enough to be considered non-isolated
      template <typename Point>
      bool closeEnough(Point const& A, Point const& B) const;
      
      
      /// Helper function. Returns a string `"(<from> to <to>)"`
      static std::string rangeString(Coord_t from, Coord_t to);
      
      /// Helper function. Returns a string `"(<from> to <to>)"`
      static std::string rangeString(Range_t range)
        { return rangeString(range.lower, range.upper); }
      
    }; // class PointIsolationAlg
    
    
    //--------------------------------------------------------------------------
    
    /// Implementation detail namespace (content is not documented)
    namespace details {
      /// Base for PositionExtractor on random-access containers
      template <typename Cont, typename Data>
      struct PositionExtractorFromArray {
        static Data x(Cont const& p) { return p[0]; }
        static Data y(Cont const& p) { return p[1]; }
        static Data z(Cont const& p) { return p[2]; }
      }; // PositionExtractorFromArray<T*>
      
      template <typename Point>
      auto extractPositionX(Point const& point)
        { return PositionExtractor<Point>::x(point); }
      template <typename Point>
      auto extractPositionY(Point const& point)
        { return PositionExtractor<Point>::y(point); }
      template <typename Point>
      auto extractPositionZ(Point const& point)
        { return PositionExtractor<Point>::z(point); }
      
    } // namespace details
    
    /// Specialisation of PositionExtractor for C array: { x, y, z }
    template <typename T>
    struct PositionExtractor<T*>:
      public details::PositionExtractorFromArray<T*, T>
    {};
    
    /// Specialisation of PositionExtractor for C++ array: { x, y, z }
    template <typename T>
    struct PositionExtractor<std::array<T, 3U>>:
      public details::PositionExtractorFromArray<std::array<T, 3U>, T>
    {};
    
    /// Specialisation of PositionExtractor for C++ vector: { x, y, z }
    /// (size is not checked!)
    template <typename T>
    struct PositionExtractor<std::vector<T>>:
      public details::PositionExtractorFromArray<std::vector<T>, T>
    {};
    
        
  } // namespace example
} // namespace lar


//--------------------------------------------------------------------------
//--- template implementation
//--------------------------------------------------------------------------

template <typename Coord>
bool lar::example::PointIsolationAlg<Coord>::Range_t::contains
  (Coord_t c) const
  { return (lower <= c) && (upper >= c); }

template <typename Coord>
bool lar::example::PointIsolationAlg<Coord>::Range_t::empty() const
  { return lower == upper; }


template <typename Coord>
bool lar::example::PointIsolationAlg<Coord>::Range_t::valid() const
  { return lower <= upper; }

template <typename Coord>
Coord lar::example::PointIsolationAlg<Coord>::Range_t::size() const
  { return upper - lower; }

template <typename Coord>
bool lar::example::PointIsolationAlg<Coord>::Range_t::operator==
  (const Range_t& as) const
  { return (upper == as.upper) && (lower == as.lower); }

template <typename Coord>
bool lar::example::PointIsolationAlg<Coord>::Range_t::operator!=
  (const Range_t& than) const
  { return (upper != than.upper) || (lower != than.lower); }


//--------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter>
std::vector<size_t> lar::example::PointIsolationAlg<Coord>::removeIsolatedPoints
  (PointIter begin, PointIter end) const
{
  //
  // reference implementation: brute force dumb one
  //
  
  std::vector<size_t> nonIsolated;
  
  size_t i = 0;
  for (auto it = begin; it != end; ++it, ++i) {
    
    for (auto ioth = begin; ioth != end; ++ioth) {
      if (it == ioth) continue;
      
      if (closeEnough(*it, *ioth)) {
        nonIsolated.push_back(i);
        break;
      }
      
    } // for oth
    
  } // for (it)
  
  return nonIsolated;
} // lar::example::PointIsolationAlg::removeIsolatedPoints()


//--------------------------------------------------------------------------
template <typename Coord>
void lar::example::PointIsolationAlg<Coord>::validateConfiguration
  (Configuration_t const& config)
{
  std::vector<std::string> errors;
  if (config.radius2 < Coord_t(0)) {
    errors.push_back
      ("invalid radius squared (" + std::to_string(config.radius2) + ")");
  }
  if (!config.rangeX.valid()) {
    errors.push_back("invalid x range " + rangeString(config.rangeX));
  }
  if (!config.rangeY.valid()) {
    errors.push_back("invalid y range " + rangeString(config.rangeY));
  }
  if (!config.rangeZ.valid()) {
    errors.push_back("invalid z range " + rangeString(config.rangeZ));
  }
  
  if (errors.empty()) return;
  
  // compose the full error message as concatenation:
  std::string message
    (std::to_string(errors.size()) + " configuration errors found:");
  
  for (auto const& error: errors) message += "\n * " + error;
  throw std::runtime_error(message);
  
} // lar::example::PointIsolationAlg::validateConfiguration()


//--------------------------------------------------------------------------
template <typename Coord>
template <typename Point>
bool lar::example::PointIsolationAlg<Coord>::closeEnough
  (Point const& A, Point const& B) const
{
  return cet::sum_of_squares(
    details::extractPositionX(A) - details::extractPositionX(B),
    details::extractPositionY(A) - details::extractPositionY(B),
    details::extractPositionZ(A) - details::extractPositionZ(B)
    ) <= config.radius2;
} // lar::example::PointIsolationAlg<Point>::closeEnough()


//--------------------------------------------------------------------------
template <typename Coord>
std::string lar::example::PointIsolationAlg<Coord>::rangeString
  (Coord_t from, Coord_t to)
  { return "(" + std::to_string(from) + " to " + std::to_string(to) + ")"; }


//--------------------------------------------------------------------------

#endif // LAREXAMPLES_ALGORITHMS_REMOVEISOLATEDSPACEPOINTS_POINTISOLATIONALG_H
