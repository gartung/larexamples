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
#include <cassert> // assert()
#include <cmath> // std::sqrt()
#include <vector>
#include <array>
#include <string>
#include <type_traits> // std::add_const_t<>
#include <iterator> // std::cbegin(), std::cend(), std::distance()
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
    
    namespace details {
      template <typename Point>
      struct PointTraits_t;
      
      /// type of Point coordinate
      template <typename Point>
      using ExtractCoordType_t
        = typename details::PointTraits_t<Point>::Coord_t;
      
    } // namespace details
    
    
    /// Index manager for a container of data arranged on a 3D array
    class GridContainerIndices {
        public:
      
      /// type of index for direct access to the cell
      using CellIndex_t = std::size_t;
      
      /// type of difference between indices
      using CellIndexOffset_t = std::ptrdiff_t;
      
      /// type of difference between indices along a dimension
      using CellDimIndex_t = CellIndexOffset_t;
      
      /// type of cell coordinate (x, y, z)
      using CellID_t = std::array<CellDimIndex_t, 3>;
      
      /// Constructor: specifies the size of the container and allocates it
      GridContainerIndices(std::array<size_t, 3> const& new_dims)
        : dims(new_dims)
        , sizes{ dims[0] * dims[1] * dims[2], dims[1] * dims[2], dims[2] }
        {}
      
      /// @{
      /// @name Grid structure
      
      /// Returns whether the specified index is valid
      bool has(CellIndexOffset_t index) const
        { return (index >= 0) && (index < (CellIndexOffset_t) size()); }
      
      /// Returns whether the specified x index is valid
      bool hasX(CellDimIndex_t index) const
        { return (index >= 0) && (index < (CellDimIndex_t) sizeX()); }
      
      /// Returns whether the specified y index is valid
      bool hasY(CellDimIndex_t index) const
        { return (index >= 0) && (index < (CellDimIndex_t) sizeY()); }
      
      /// Returns whether the specified z index is valid
      bool hasZ(CellDimIndex_t index) const
        { return (index >= 0) && (index < (CellDimIndex_t) sizeZ()); }
      
      /// Returns the number of cells in the grid
      size_t size() const { return sizes[0]; }
      
      /// Returns the number of cells on the x axis of the grid
      size_t sizeX() const { return dims[0]; }
      
      /// Returns the number of cells on the y axis of the grid
      size_t sizeY() const { return dims[1]; }
      
      /// Returns the number of cells on the z axis of the grid
      size_t sizeZ() const { return dims[2]; }
      
      /// @}
      
      /// @{
      /// @name Indexing
      
      /// Returns the index of the element from its cell coordinates (no check!)
      CellIndex_t operator[] (CellID_t id) const { return index(id); }
      
      /// Returns the difference in index of cellID respect to origin
      CellIndexOffset_t offset
        (CellID_t const& origin, CellID_t const& cellID) const
        { return index(cellID) - index(origin); }
      
      /// @}
      
        protected:
      
      ///< cells on each of the dimensions of the grid ({ x, y, z })
      std::array<size_t, 3> dims;
      
      ///< size of each level of the grid (0: x*y*z, 1: y*z, 2: z)
      std::array<size_t, 3> sizes;
      
      /// Returns the index of the element from its cell coordinates (no check!)
      CellIndex_t index(CellID_t id) const
        { return CellIndex_t(id[0] * sizes[1] + id[1] * sizes[2] + id[2]); }
      
    }; // GridContainerIndices
      
    
    /// Container of data arranged on a 3D grid
    template <typename Datum>
    class GridContainer {
      
        public:
      using Datum_t = Datum; ///< type of contained datum
      using Grid_t = GridContainer<Datum_t>; ///< this type
      
      /// type of index manager
      using Indexer_t = GridContainerIndices;
      
      /// type of index for direct access to the cell
      using CellIndex_t = Indexer_t::CellIndex_t;
      
      /// type of difference between indices
      using CellIndexOffset_t = Indexer_t::CellIndexOffset_t;
      
      /// type of difference between indices
      using CellDimIndex_t = Indexer_t::CellDimIndex_t;
      
      /// type of cell coordinate (x, y, z)
      using CellID_t = Indexer_t::CellID_t;
      
      /// type of a single cell container
      using Cell_t = std::vector<Datum_t>;
      
      /// type of container holding all cells
      using Cells_t = std::vector<Cell_t>;
      
      /// type of iterator to all cells
      using const_iterator = typename Cells_t::const_iterator;
      
      /// Constructor: specifies the size of the container and allocates it
      GridContainer(std::array<size_t, 3> const& dims)
        : indices(dims)
        , data(indices.size())
        {}
      
      /// @{
      /// @name Data structure
      
      /// Returns whether the specified index is valid
      bool has(CellIndexOffset_t index) const { return indices.has(index); }
      
      /// Returns whether the specified x index is valid
      bool hasX(CellDimIndex_t index) const { return indices.hasX(index); }
      
      /// Returns whether the specified x index is valid
      bool hasY(CellDimIndex_t index) const { return indices.hasY(index); }
      
      /// Returns whether the specified x index is valid
      bool hasZ(CellDimIndex_t index) const { return indices.hasZ(index); }
      
      /// @}
      
      /// @{
      /// @name Data access
      
      /// Return the index of the element from its cell coordinates (no check!)
      CellIndex_t index(CellID_t const& id) const { return indices[id]; }
      
      /// Returns the difference in index from two cells
      CellIndexOffset_t indexOffset
        (CellID_t const& origin, CellID_t const& cellID) const
        { return indices.offset(origin, cellID); }
      
      /// Returns a reference to the specified cell
      Cell_t& operator[] (CellID_t const& id) { return cell(id); }
      
      /// Returns a constant reference to the specified cell
      Cell_t const& operator[] (CellID_t const& id) const { return cell(id); }
      
      /// Returns a reference to to the cell with specified index
      Cell_t& operator[] (CellIndex_t index) { return data[index]; }
      
      /// Returns a constant reference to the cell with specified index
      Cell_t const& operator[] (CellIndex_t index) const { return data[index]; }
      
      ///@}
      
      /// @{
      /// @name Data insertion
      
      /// Copies an element into the specified cell
      void insert(CellID_t const& cellID, Datum_t const& elem)
        { cell(cellID).push_back(elem); }
      
      /// Moves an element into the specified cell
      void insert(CellID_t const& cellID, Datum_t&& elem)
        { cell(cellID).push_back(std::move(elem)); }
      
      /// Copies an element into the cell with the specified index
      void insert(CellIndex_t index, Datum_t const& elem)
        { data[index].push_back(elem); }
      
      /// Moves an element into the cell with the specified index
      void insert(CellIndex_t index, Datum_t&& elem)
        { data[index].push_back(std::move(elem)); }
      
      /// @}
      
      /// Returns the index manager of the grid
      Indexer_t const& indexManager() const { return indices; }
      
      
        protected:
      Indexer_t indices; ///< manager of the indices of the container
      
      Cells_t data; ///< organised collection of points
      
      /// Returns a reference to the specified cell
      Cell_t& cell(CellID_t const& cellID)
        { return data[index(cellID)]; }
      
      /// Returns a constant reference to the specified cell
      Cell_t const& cell(CellID_t const& cellID) const
        { return data[index(cellID)]; }
      
    }; // GridContainer<>
      
    
    /// Range of coordinates
    template <typename Coord>
    struct CoordRange {
      using Range_t = CoordRange<Coord>; ///< this type
      using Coord_t = Coord; ///< data type for coordinate
      
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
      
      /// Returns the distance of the specified coordinate from the lower bound
      Coord_t offset(Coord_t c) const { return c - lower; }
      
      /// Returns whether the specified range has the same limits as this
      bool operator== (const Range_t& as) const;
      
      /// Returns whether the specified range has limits different than this
      bool operator!= (const Range_t& than) const;
      
    }; // CoordRange<>
    
    
    /// A container of points sorted in cells
    /// @tparam PointIter type of iterator to the point
    template <typename PointIter>
    class SpacePartition {
      using Point_t = decltype(*(PointIter())); ///< type of the point
      using Grid_t = GridContainer<PointIter>; ///< base class
      
        public:
      /// type of point coordinate
      using Coord_t = details::ExtractCoordType_t<Point_t>;
      using Range_t = CoordRange<Coord_t>; ///< type of coordinate range
      
      /// type of index manager of the grid
      using Indexer_t = typename Grid_t::Indexer_t;
      
      /// type of difference between cell indices
      using CellIndexOffset_t = typename Indexer_t::CellIndexOffset_t;
      
      using CellIndex_t = typename Indexer_t::CellIndex_t; /// type of cell index
      
      /// type of cell identifier
      using CellID_t = typename Indexer_t::CellID_t;
      
      /// type of cell
      using Cell_t = typename Grid_t::Cell_t;
      
      /// Constructs the partition in a given volume with the given cell size
      SpacePartition
        (Range_t rangeX, Range_t rangeY, Range_t rangeZ, Coord_t cellSide);
      
      /// Fills the partition with the points in the specified range
      /// @throw std::runtime_error a point is outside the covered volume
      void fill(PointIter begin, PointIter end);
      
      /// Returns the index pertaining the point (might be invalid!)
      /// @throw std::runtime_error point is outside the covered volume
      CellIndexOffset_t pointIndex(Point_t const& point) const;
      
      /// Returns the index manager of the grid
      Indexer_t const& indexManager() const
        { return data.indexManager(); }
      
      /// Returns whether there is a cell with the specified index (signed!)
      bool has(CellIndexOffset_t ofs) const
        { return data.has(ofs); }
      
      /// Returns the cell with the specified index
      Cell_t const& operator[] (CellIndex_t index) const
        { return data[index]; }
      
      /// Returns a constant iterator pointing to the first cell
      typename Grid_t::const_iterator begin() const;
      
      /// Returns a constant iterator pointing after the last cell
      typename Grid_t::const_iterator end() const;
      
        protected:
      using CellDimIndex_t = typename Grid_t::CellDimIndex_t;
      
      Coord_t cellSize; ///< length of the side of each cubic cell
      
      Range_t xRange; ///< coordinates of the contained volume on x axis
      Range_t yRange; ///< coordinates of the contained volume on z axis
      Range_t zRange; ///< coordinates of the contained volume on z axis
      
      Grid_t data; ///< container of points
      
      /// Returns the number of cell for a coordinate c in the specified range
      CellDimIndex_t cellNumber(Coord_t c, Range_t const& range) const
        { return CellDimIndex_t(range.offset(c) / cellSize); }
      
    }; // SpacePartition<>
      
      
    
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
     * 
     * 
     * Description of the algorithm
     * -----------------------------
     * 
     * The basic method to determine the isolation of a point is by brute force,
     * by computing the distance with all others and, as soon as one of them is
     * found too close, declare the point non-isolated.
     * 
     * A refinement is implemented: the points are grouped in cubic "cells"
     * and points in cells that are farther than isolation radius are not
     * checked against each other. This requires some memory to allocate the
     * structure, that can become huge. The maximum memory parameter keeps this
     * sane.
     * 
     * Other refinements are not implemented. When a point is found non-isolated
     * also the point that makes it non-isolated should also be marked so. Cell
     * radius might be tuned to be smaller. Some of the neighbour cells may be
     * too far and should not be checked. The grid allocates a vector for each
     * cell, whether it's empty or not; using a sparse structure might reduce
     * the memory; also if the grid contains pointers to vectors instead of
     * vectors, and the grid is very sparse, there should still be some memory
     * saving.
     * 
     */
    template <typename Coord = double>
    class PointIsolationAlg {
      
        public:
      /// Type of coordinate
      using Coord_t = Coord;
      using Range_t = CoordRange<Coord_t>;
      
      /// Type containing all configuration parameters of the algorithm
      struct Configuration_t {
        Range_t rangeX;   ///< range in X of the covered volume
        Range_t rangeY;   ///< range in Y of the covered volume
        Range_t rangeZ;   ///< range in Z of the covered volume
        Coord_t radius2;  ///< square of isolation radius [cm^2]
        size_t maxMemory = 100 * 1048576;
                          ///< grid smaller than this number of bytes (100 MiB)
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
      
      
      /// Returns the maximum optimal cell size when using a isolation radius
      static Coord_t maximumOptimalCellSize(Coord_t radius)
        { return radius * 2. / std::sqrt(3.); }
      
      
        private:
      /// type managing cell indices
      using Indexer_t = GridContainerIndices;
      
      /// type of neighbourhood cell offsets
      using NeighAddresses_t = std::vector<Indexer_t::CellIndexOffset_t>;
      
      template <typename PointIter>
      using Partition_t = SpacePartition<PointIter>;
      
      template <typename PointIter>
      using Point_t = decltype(*PointIter());
      
      
      Configuration_t config; ///< all configuration data
      
        
      /// Computes the cell size to be used
      template <typename PointIter = std::array<double, 3> const*>
      Coord_t computeCellSize() const;
      
      
      /// Returns a list of cell offsets for the neighbourhood of given radius
      NeighAddresses_t buildNeighborhood
        (Indexer_t const& indexer, unsigned int neighExtent) const;
      
      /// Returns whether a point is isolated with respect to all the others
      template <typename PointIter>
      bool isPointIsolatedFrom(
        Point_t<PointIter> const& point,
        typename Partition_t<PointIter>::Cell_t const& otherPoints
        ) const;
        
      /// Returns whether a point is isolated in the specified neighbourhood
      template <typename PointIter>
      bool isPointIsolatedWithinNeighborhood(
        Partition_t<PointIter> const& partition,
        Indexer_t::CellIndex_t cellIndex,
        Point_t<PointIter> const& point,
        NeighAddresses_t const& neighList
        ) const;

      /// Brute-force reference algorithm
      template <typename PointIter>
      std::vector<size_t> bruteRemoveIsolatedPoints
        (PointIter begin, PointIter end) const;
      
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
      
      template <typename Point>
      struct PointTraits_t {
        /// type of Point coordinate
        using Coord_t = decltype(extractPositionX(std::decay_t<Point>()));
      }; // ExtractCoordTypeHelper_t
    
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

// 
//------------------------------------------------------------------------------
//--- lar::example::details
//---
namespace lar {
  namespace example {
    namespace details {
      
      /// Returns the dimensions of a grid diced with the specified size
      template <typename Coord>
      std::array<size_t, 3> diceVolume(
        CoordRange<Coord> const& rangeX,
        CoordRange<Coord> const& rangeY,
        CoordRange<Coord> const& rangeZ,
        Coord diceSize
        )
        {
          return {
            size_t(std::ceil(rangeX.size() / diceSize)),
            size_t(std::ceil(rangeY.size() / diceSize)),
            size_t(std::ceil(rangeZ.size() / diceSize))
            };
        } // diceVolume()
      
    } // namespace details
  } // namespace example
} // namespace lar


//------------------------------------------------------------------------------
//--- template implementation
//------------------------------------------------------------------------------
//--- lar::example::CoordRange
//---
template <typename Coord>
bool lar::example::CoordRange<Coord>::contains(Coord_t c) const
  { return (lower <= c) && (upper >= c); }

template <typename Coord>
bool lar::example::CoordRange<Coord>::Range_t::empty() const
  { return lower == upper; }


template <typename Coord>
bool lar::example::CoordRange<Coord>::valid() const
  { return lower <= upper; }

template <typename Coord>
Coord lar::example::CoordRange<Coord>::size() const
  { return upper - lower; }

template <typename Coord>
bool lar::example::CoordRange<Coord>::operator== (const Range_t& as) const
  { return (upper == as.upper) && (lower == as.lower); }

template <typename Coord>
bool lar::example::CoordRange<Coord>::operator!= (const Range_t& than) const
  { return (upper != than.upper) || (lower != than.lower); }


//------------------------------------------------------------------------------
//--- lar::example::SpacePartition
//---
template <typename PointIter>
lar::example::SpacePartition<PointIter>::SpacePartition
  (Range_t rangeX, Range_t rangeY, Range_t rangeZ, Coord_t cellSide)
  : cellSize(cellSide)
  , xRange(rangeX)
  , yRange(rangeY)
  , zRange(rangeZ)
  , data(details::diceVolume(xRange, yRange, zRange, cellSize))
{
//   std::cout << "Cell size: " << cellSize << "; grid: "
//     << indexManager().sizeX() << " x "
//     << indexManager().sizeY() << " x " << indexManager().sizeZ()
//     << " (" << indexManager().size() << " cells)"
//     << "\n  range X: " << xRange.lower << " -- " << xRange.upper
//     << "\n  range Y: " << yRange.lower << " -- " << yRange.upper
//     << "\n  range Z: " << zRange.lower << " -- " << zRange.upper
//     << std::endl;
} // lar::example::SpacePartition<>::SpacePartition


//--------------------------------------------------------------------------
template <typename PointIter>
void lar::example::SpacePartition<PointIter>::fill
  (PointIter begin, PointIter end)
{
  
  PointIter it = begin;
  while (it != end) {
    // if the point is outside the volume, pointIndex will throw an exception
    data.insert(pointIndex(*it), it);
    ++it;
  } // while
  
} // lar::example::SpacePartition<>::fill()


//--------------------------------------------------------------------------
template <typename PointIter>
typename lar::example::SpacePartition<PointIter>::CellIndexOffset_t
lar::example::SpacePartition<PointIter>::pointIndex(Point_t const& point) const
{
  // compute the cell ID coordinates
  Coord_t const x = details::extractPositionX(point);
  CellDimIndex_t const xc = cellNumber(x, xRange);
  if (!data.hasX(xc)) {
    throw std::runtime_error
      ("Point out of the volume (x = " + std::to_string(x) + ")");
  }
  
  Coord_t const y = details::extractPositionY(point);
  CellDimIndex_t const yc = cellNumber(y, yRange);
  if (!data.hasY(yc)) {
    throw std::runtime_error
      ("Point out of the volume (y = " + std::to_string(y) + ")");
  }
  
  Coord_t const z = details::extractPositionZ(point);
  CellDimIndex_t const zc = cellNumber(z, zRange);
  if (!data.hasZ(zc)) {
    throw std::runtime_error
      ("Point out of the volume (z = " + std::to_string(z) + ")");
  }
  
  // return its index
  return data.index(CellID_t{ xc, yc, zc });
  
} // lar::example::SpacePartition<>::pointIndex()


//------------------------------------------------------------------------------
//--- lar::example::PointIsolationAlg
//------------------------------------------------------------------------------
template <typename Coord>
typename lar::example::PointIsolationAlg<Coord>::NeighAddresses_t
lar::example::PointIsolationAlg<Coord>::buildNeighborhood
  (Indexer_t const& indexer, unsigned int neighExtent) const
{
  unsigned int const neighSize = 1 + 2 * neighExtent;
  NeighAddresses_t neighList;
  neighList.reserve(neighSize * neighSize * neighSize - 1);
  
  using CellID_t = Indexer_t::CellID_t;
  using CellDimIndex_t = Indexer_t::CellDimIndex_t;
  
  //
  // optimisation (speed): reshape the neighbourhood
  // neighbourhood might cut out cells close to the vertices
  //
  
  // TODO
  
  CellDimIndex_t const ext = neighExtent; // convert into the right signedness
  
  CellID_t center{ 0, 0, 0 }, cellID;
  for (CellDimIndex_t ixOfs = -ext; ixOfs <= ext; ++ixOfs) {
    cellID[0] = ixOfs;
    for (CellDimIndex_t iyOfs = -ext; iyOfs <= ext; ++iyOfs) {
      cellID[1] = iyOfs;
      for (CellDimIndex_t izOfs = -ext; izOfs <= ext; ++izOfs) {
        if ((ixOfs == 0) && (iyOfs == 0) && (izOfs == 0)) continue;
        cellID[2] = izOfs;
        
        neighList.push_back(indexer.offset(center, cellID));
        
      } // for ixOfs
    } // for iyOfs
  } // for izOfs
  
  return neighList;
} // lar::example::PointIsolationAlg<Coord>::buildNeighborhood()

  
//--------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter>
bool lar::example::PointIsolationAlg<Coord>::isPointIsolatedFrom(
  Point_t<PointIter> const& point,
  typename Partition_t<PointIter>::Cell_t const& otherPoints
) const
{
  
  for (auto const& otherPointPtr: otherPoints) {
    // make sure that we did not compare the point with itself
    if (closeEnough(point, *otherPointPtr) && (&point != &*otherPointPtr))
      return false;
  }
  
  return true;
  
} // lar::example::PointIsolationAlg<Coord>::isPointIsolatedFrom()


//--------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter>
bool lar::example::PointIsolationAlg<Coord>::isPointIsolatedWithinNeighborhood(
  Partition_t<PointIter> const& partition,
  Indexer_t::CellIndex_t cellIndex,
  Point_t<PointIter> const& point,
  NeighAddresses_t const& neighList
) const
{
  
  // check in all cells of the neighbourhood
  for (Indexer_t::CellIndexOffset_t neighOfs: neighList) {
    
    //
    // optimisation (speed): have neighbour offsets so that the invalid ones
    // are all at the beginning and at the end, so that skipping is faster
    //
    
    if (!partition.has(cellIndex + neighOfs)) continue;
    auto const& neighCellPoints = partition[cellIndex + neighOfs];

    if (!isPointIsolatedFrom<PointIter>(point, neighCellPoints)) return false;
    
  } // for neigh cell
  
  return true;
  
} // lar::example::PointIsolationAlg<Coord>::isPointIsolatedWithinNeighborhood()


//------------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter>
std::vector<size_t> lar::example::PointIsolationAlg<Coord>::removeIsolatedPoints
  (PointIter begin, PointIter end) const
{
  
  std::vector<size_t> nonIsolated;
  
  Coord_t const R = std::sqrt(config.radius2);
  
  using Point_t = decltype(*begin);

  //
  // determine space partition settings: cell size
  // 
  // maximum: the volume of a single cell must be contained in a sphere with
  // radius equal to the isolation radius R
  // 
  // minimum: needs tuning
  // 
  
  Coord_t cellSize = computeCellSize<PointIter>();
  assert(cellSize > 0);
  Partition_t<PointIter> partition
    (config.rangeX, config.rangeY, config.rangeZ, cellSize);
  
  // if a cell is contained in a sphere with 
  bool const cellContainedInIsolationSphere
    = (cellSize <= maximumOptimalCellSize(R));
  
  // 
  // determine neighbourhood
  // the neighbourhood is the number of cells that might contain points closer
  // than R to a cell; it is equal to R in cell size units, rounded up;
  // it's expressed as a list of coordinate shifts from a base cell to all the
  // others in the neighbourhood; it is contained in a cube
  //
  unsigned int const neighExtent = (int) std::ceil(R / cellSize);
  NeighAddresses_t neighList
    = buildNeighborhood(partition.indexManager(), neighExtent);
  
  // if a cell is not fully contained in a isolation radius, we need to check
  // the points of the cell with each other: their cell becomes part of the
  // neighbourhood
  if (!cellContainedInIsolationSphere)
    neighList.insert(neighList.begin(), { 0, 0, 0 });
  
  //
  // populate the partition
  //
  partition.fill(begin, end);
  
  //
  // for each cell in the partition:
  //
  size_t const nCells = partition.indexManager().size();
  for (Indexer_t::CellIndex_t cellIndex = 0; cellIndex < nCells; ++cellIndex) {
    auto const& cellPoints = partition[cellIndex];
    
    //
    // if the cell has more than one element, mark all points as non-isolated;
    // true only if the cell is completely contained within a R rßadius
    //
    if (cellContainedInIsolationSphere && (cellPoints.size() > 1)) {
      for (auto const& pointPtr: cellPoints)
        nonIsolated.push_back(std::distance(begin, pointPtr));
      continue;
    } // if all non-isolated
    
    //
    // brute force approach: try all the points in this cell against all the
    // points in the neighbourhood
    //
    for (auto const pointPtr: cellPoints) {
      //
      // optimisation (speed): mark the points from other cells as non-isolated
      // when they trigger non-isolation in points of the current one
      //
      
      // TODO
      
      if (!isPointIsolatedWithinNeighborhood
        (partition, cellIndex, *pointPtr, neighList)
        )
      {
        nonIsolated.push_back(std::distance(begin, pointPtr));
      }
    } // for points in cell
    
  } // for cell
  
  return nonIsolated;
} // lar::example::PointIsolationAlg::removeIsolatedPoints()


//--------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter>
std::vector<size_t>
lar::example::PointIsolationAlg<Coord>::bruteRemoveIsolatedPoints
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
} // lar::example::PointIsolationAlg::bruteRemoveIsolatedPoints()


//--------------------------------------------------------------------------
template <typename Coord>
template <typename PointIter /* = std::array<double, 3> const* */>
Coord lar::example::PointIsolationAlg<Coord>::computeCellSize() const {
  
  Coord_t const R = std::sqrt(config.radius2);
  
  // maximum: the volume of a single cell must be contained in a sphere with
  //   radius equal to the isolation radius R;
  // minimum: needs tuning
  Coord_t cellSize = maximumOptimalCellSize(R); // try the minimum for now

  //
  // optimisation (memory): determine minimum size of box
  //
  
  // TODO
  
  if (config.maxMemory == 0) return cellSize;
  
  do {
    std::array<size_t, 3> partition = details::diceVolume
      (config.rangeX, config.rangeY, config.rangeZ, cellSize);
    
    size_t const nCells = partition[0] * partition[1] * partition[2];
    if (nCells <= 1) break; // we can't reduce it any further
    
    // is memory low enough?
    size_t const memory
      = nCells * sizeof(typename SpacePartition<PointIter>::Cell_t);
    if (memory < config.maxMemory) break;
    
    cellSize *= 2;
  } while (true);
  
  return cellSize;
} // lar::example::PointIsolationAlg<Coord>::computeCellSize()


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
