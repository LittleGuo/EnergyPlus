// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef PlantPipingSystemsManager_hh_INCLUDED
#define PlantPipingSystemsManager_hh_INCLUDED

// C++ Headers
#include <map>
#include <memory>
#include <utility>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <DataGlobals.hh>
#include <EnergyPlus.hh>
#include <GroundTemperatureModeling/GroundTemperatureModelManager.hh>
#include <PlantComponent.hh>

namespace EnergyPlus {

namespace PlantPipingSystemsManager {

    // MODULE PARAMETER DEFINITIONS:
    extern std::string const ObjName_ug_GeneralDomain;
    extern std::string const ObjName_Circuit;
    extern std::string const ObjName_Segment;
    extern std::string const ObjName_HorizTrench;
    extern std::string const ObjName_ZoneCoupled_Slab;
    extern std::string const ObjName_ZoneCoupled_Basement;

    // Using/Aliasing
    using namespace GroundTemperatureManager;

    enum class SegmentFlow
    {
        IncreasingZ,
        DecreasingZ
    };
    enum class MeshDistribution
    {
        Uniform,
        SymmetricGeometric,
        Geometric
    };
    enum class RegionType
    {
        Pipe,
        BasementWall,
        BasementFloor,
        XDirection,
        YDirection,
        ZDirection,
        XSide,
        XSideWall,
        ZSide,
        ZSideWall,
        FloorInside,
        UnderFloor,
        HorizInsXSide,
        HorizInsZSide,
        VertInsLowerEdge
    };
    enum class Direction
    {
        PositiveY,
        NegativeY,
        PositiveX,
        NegativeX,
        PositiveZ,
        NegativeZ
    };
    enum class PartitionType
    {
        BasementWall,
        BasementFloor,
        Pipe,
        Slab,
        XSide,
        XSideWall,
        ZSide,
        ZSideWall,
        FloorInside,
        UnderFloor,
        HorizInsXSide,
        VertInsLowerEdge,
        HorizInsZSide
    };
    enum class CellType
    {
        Unknown,
        Pipe,
        GeneralField,
        GroundSurface,
        FarfieldBoundary,
        BasementWall,
        BasementFloor,
        BasementCorner,
        BasementCutaway,
        Slab,
        HorizInsulation,
        VertInsulation,
        ZoneGroundInterface
    };

    struct BaseThermalPropertySet
    {
        // Members
        Real64 Conductivity; // W/mK
        Real64 Density;      // kg/m3
        Real64 SpecificHeat; // J/kgK

        // Default Constructor
        BaseThermalPropertySet() : Conductivity(0.0), Density(0.0), SpecificHeat(0.0)
        {
        }

        // Member Constructor
        BaseThermalPropertySet(Real64 const Conductivity, // W/mK
                               Real64 const Density,      // kg/m3
                               Real64 const SpecificHeat  // J/kgK
                               )
            : Conductivity(Conductivity), Density(Density), SpecificHeat(SpecificHeat)
        {
        }

        Real64 inline diffusivity()
        {
            return this->Conductivity / (this->Density * this->SpecificHeat);
        }
    };

    struct ExtendedFluidProperties : BaseThermalPropertySet
    {
        // Members
        Real64 Viscosity = 0.0; // kg/m-s
        Real64 Prandtl = 0.0;   // -

        // Default Constructor
        ExtendedFluidProperties() = default;

        // Member Constructor
        ExtendedFluidProperties(Real64 const Conductivity, // W/mK
                                Real64 const Density,      // kg/m3
                                Real64 const SpecificHeat, // J/kgK
                                Real64 const Viscosity,    // kg/m-s
                                Real64 const Prandtl       // -
                                )
            : BaseThermalPropertySet(Conductivity, Density, SpecificHeat), Viscosity(Viscosity), Prandtl(Prandtl)
        {
        }
    };

    struct BaseCell
    {
        // Members
        Real64 Temperature;               // C
        Real64 Temperature_PrevIteration; // C
        Real64 Temperature_PrevTimeStep;  // C
        Real64 Beta;                      // K/W
        BaseThermalPropertySet Properties;

        // Default Constructor
        BaseCell() : Temperature(0.0), Temperature_PrevIteration(0.0), Temperature_PrevTimeStep(0.0), Beta(0.0)
        {
        }
    };

    struct RadialSizing
    {
        // Members
        Real64 InnerDia = 0.0;
        Real64 OuterDia = 0.0;

        // Default Constructor
        RadialSizing() = default;

        Real64 inline thickness()
        {
            return (this->OuterDia - this->InnerDia) / 2.0;
        }
    };

    struct RadialCellInformation : BaseCell
    {
        // Members
        Real64 RadialCentroid = 0.0;
        Real64 InnerRadius = 0.0;
        Real64 OuterRadius = 0.0;

        // Default Constructor
        RadialCellInformation() = default;

        // Member Constructor
        RadialCellInformation(Real64 const m_RadialCentroid, Real64 const m_MinRadius, Real64 const m_MaxRadius)
        {
            RadialCentroid = m_RadialCentroid;
            InnerRadius = m_MinRadius;
            OuterRadius = m_MaxRadius;
        }

        // Get the XY cross sectional area of the radial cell
        Real64 inline XY_CrossSectArea()
        {
            return DataGlobals::Pi * (pow_2(this->OuterRadius) - pow_2(this->InnerRadius));
        }
    };

    struct FluidCellInformation : BaseCell
    {
        // Members
        Real64 Volume = 0.0;
        ExtendedFluidProperties Properties;

        // Default Constructor
        FluidCellInformation() = default;

        // Member Constructor
        FluidCellInformation(Real64 const m_PipeInnerRadius, Real64 const m_CellDepth)
        {
            this->Volume = DataGlobals::Pi * pow_2(m_PipeInnerRadius) * m_CellDepth;
        }
    };

    struct CartesianPipeCellInformation // Specialized cell information only used by cells which contain pipes
    {
        // Members
        Array1D<RadialCellInformation> Soil;
        RadialCellInformation Insulation;
        RadialCellInformation Pipe;
        FluidCellInformation Fluid;
        Real64 RadialSliceWidth = 0.0;
        Real64 InterfaceVolume = 0.0;

        // Default Constructor
        CartesianPipeCellInformation() = default;

        // Eventually this should be the real constructor
        static void ctor(CartesianPipeCellInformation &c,
                         Real64 GridCellWidth,
                         RadialSizing const &PipeSizes,
                         int NumRadialNodes,
                         Real64 CellDepth,
                         Real64 InsulationThickness,
                         Real64 RadialGridExtent,
                         bool SimHasInsulation);
    };

    struct Point
    {
        // Members
        int X = 0;
        int Y = 0;

        // Default Constructor
        Point() = default;

        // Member Constructor
        Point(int const X, int const Y) : X(X), Y(Y)
        {
        }
    };

    struct PointF
    {
        // Members
        Real64 X = 0.0;
        Real64 Y = 0.0;

        // Default Constructor
        PointF() = default;

        // Member Constructor
        PointF(Real64 const X, Real64 const Y) : X(X), Y(Y)
        {
        }
    };

    struct Point3DInteger
    {
        // Members
        int X = 0;
        int Y = 0;
        int Z = 0;

        // Default Constructor
        Point3DInteger() = default;

        // Member Constructor
        Point3DInteger(int const X, int const Y, int const Z) : X(X), Y(Y), Z(Z)
        {
        }
    };

    struct Point3DReal
    {
        // Members
        Real64 X = 0.0;
        Real64 Y = 0.0;
        Real64 Z = 0.0;

        // Default Constructor
        Point3DReal() = default;

        // Member Constructor
        Point3DReal(Real64 const X, Real64 const Y, Real64 const Z) : X(X), Y(Y), Z(Z)
        {
        }
    };

    struct MeshPartition
    {
        // Members
        Real64 rDimension = 0.0;
        PartitionType partitionType = PartitionType::Pipe;
        Real64 TotalWidth = 0.0;

        // Default Constructor
        MeshPartition() = default;

        // Member Constructor
        MeshPartition(Real64 const rDimension,
                      PartitionType const partitionType, // From Enum: ParitionType
                      Real64 const TotalWidth)
            : rDimension(rDimension), partitionType(partitionType), TotalWidth(TotalWidth)
        {
        }
    };

    struct GridRegion
    {
        // Members
        Real64 Min = 0.0;
        Real64 Max = 0.0;
        RegionType thisRegionType = RegionType::Pipe;
        Array1D<Real64> CellWidths;

        // Default Constructor
        GridRegion() = default;

        // Member Constructor
        GridRegion(Real64 Min, Real64 Max, RegionType thisRegionType, Array1D<Real64> CellWidths)
            : Min(Min), Max(Max), thisRegionType(thisRegionType), CellWidths(std::move(CellWidths))
        {
        }
    };

    struct RectangleF
    {
        // Members
        Real64 X_min = 0.0;
        Real64 Y_min = 0.0;
        Real64 Width = 0.0;
        Real64 Height = 0.0;

        // Default Constructor
        RectangleF() = default;

        // Member Constructor
        RectangleF(Real64 const X_min, Real64 const Y_min, Real64 const Width, Real64 const Height)
            : X_min(X_min), Y_min(Y_min), Width(Width), Height(Height)
        {
        }

        bool inline contains(PointF const &p)
        {
            return ((this->X_min <= p.X) && (p.X < (this->X_min + this->Width)) && (this->Y_min <= p.Y) && (p.Y < (this->Y_min + this->Height)));
        }
    };

    struct NeighborInformation
    {
        // Members
        Real64 ThisCentroidToNeighborWall = 0.0;
        Real64 ThisWallToNeighborCentroid = 0.0;
        Real64 adiabaticMultiplier = 1.0;
        Direction direction = Direction::NegativeX;

        // Default Constructor
        NeighborInformation() = default;
    };

    struct CartesianCell : BaseCell
    {
        // Members
        int X_index = 0;
        int Y_index = 0;
        int Z_index = 0;
        Real64 X_min = 0.0;
        Real64 X_max = 0.0;
        Real64 Y_min = 0.0;
        Real64 Y_max = 0.0;
        Real64 Z_min = 0.0;
        Real64 Z_max = 0.0;
        Point3DReal Centroid;
        CellType cellType = CellType::Unknown;
        std::map<Direction, NeighborInformation> NeighborInfo;
        CartesianPipeCellInformation PipeCellData;

        // Default Constructor
        CartesianCell() = default;

        Real64 inline width() const
        {
            return this->X_max - this->X_min;
        }

        Real64 inline height() const
        {
            return this->Y_max - this->Y_min;
        }

        Real64 inline depth() const
        {
            return this->Z_max - this->Z_min;
        }

        Real64 inline XNormalArea() const
        {
            return this->depth() * this->height();
        }

        Real64 inline YNormalArea() const
        {
            return this->depth() * this->width();
        }

        Real64 inline ZNormalArea() const
        {
            return this->width() * this->height();
        }

        Real64 inline volume() const
        {
            return this->width() * this->depth() * this->height();
        }

        Real64 normalArea(Direction direction) const;

        void EvaluateNeighborCoordinates(Direction CurDirection, int &NX, int &NY, int &NZ);

    };

    struct MeshExtents
    {
        // Members
        Real64 xMax = 0.0;
        Real64 yMax = 0.0;
        Real64 zMax = 0.0;

        // Default Constructor
        MeshExtents() = default;

        // Member Constructor
        MeshExtents(Real64 const xMax, Real64 const yMax, Real64 const zMax) : xMax(xMax), yMax(yMax), zMax(zMax)
        {
        }
    };

    struct DistributionStructure
    {
        // Members
        MeshDistribution thisMeshDistribution = MeshDistribution::Uniform;
        int RegionMeshCount = 0;
        Real64 GeometricSeriesCoefficient = 0.0;

        // Default Constructor
        DistributionStructure() = default;
    };

    struct MeshProperties
    {
        // Members
        DistributionStructure X;
        DistributionStructure Y;
        DistributionStructure Z;

        // Default Constructor
        MeshProperties() = default;
    };

    struct SimulationControl
    {
        // Members
        Real64 MinimumTemperatureLimit = -1000;
        Real64 MaximumTemperatureLimit = 1000;
        Real64 Convergence_CurrentToPrevIteration = 0.0;
        int MaxIterationsPerTS = 0;

        // Default Constructor
        SimulationControl() = default;
    };

    struct FarfieldInfo
    {
        std::shared_ptr<BaseGroundTempsModel> groundTempModel;
    };

    struct BasementZoneInfo
    {
        // Members
        Real64 Depth = 0;  // m
        Real64 Width = 0;  // m
        Real64 Length = 0; // m
        bool ShiftPipesByWidth = false;
        std::string WallBoundaryOSCMName = "";
        int WallBoundaryOSCMIndex = 0;
        std::string FloorBoundaryOSCMName = "";
        int FloorBoundaryOSCMIndex = 0;
        std::vector<int> WallSurfacePointers;
        std::vector<int> FloorSurfacePointers;
        int BasementWallXIndex = -1;
        int BasementFloorYIndex = -1;

        // Default Constructor
        BasementZoneInfo() = default;
    };

    struct MeshPartitions
    {
        // Members
        Array1D<MeshPartition> X;
        Array1D<MeshPartition> Y;
        Array1D<MeshPartition> Z;

        // Default Constructor
        MeshPartitions() = default;
    };

    struct MoistureInfo
    {
        // Members
        Real64 Theta_liq = 0.3; // volumetric moisture content of the soil
        Real64 Theta_sat = 0.5; // volumetric moisture content of soil at saturation
        Real64 GroundCoverCoefficient = 0.408;
        Real64 rhoCP_soil_liq = 0.0;
        Real64 rhoCP_soil_transient = 0.0;
        Real64 rhoCP_soil_ice = 0.0;
        Real64 rhoCp_soil_liq_1 = 0.0;

        // Default Constructor
        MoistureInfo() = default;
    };

    struct CurSimConditionsInfo
    {
        // Members
        // Simulation conditions
        Real64 PrevSimTimeSeconds = -1.0;
        Real64 CurSimTimeSeconds = 0.0;
        Real64 CurSimTimeStepSize = 0.0;
        // Environmental conditions
        Real64 CurAirTemp = 10.0;
        Real64 CurWindSpeed = 2.6;
        Real64 CurIncidentSolar = 0.0;
        Real64 CurRelativeHumidity = 100.0;

        // Default Constructor
        CurSimConditionsInfo() = default;
    };

    struct PipeSegmentInfo
    {
        // Members
        // ID
        std::string Name = "";
        // Misc inputs
        PointF PipeLocation;
        Point PipeCellCoordinates;
        SegmentFlow FlowDirection = SegmentFlow::IncreasingZ;
        // Reporting variables
        Real64 InletTemperature = 0.0;
        Real64 OutletTemperature = 0.0;
        Real64 FluidHeatLoss = 0.0;
        // Error handling flags
        bool PipeCellCoordinatesSet = false;
        // Other flags
        bool IsActuallyPartOfAHorizontalTrench = false;

        // Default Constructor
        PipeSegmentInfo() = default;

        void initPipeCells(int x, int y);
    };

    struct PipeCircuitInfo : public PlantComponent
    {

        // Members
        // ID
        std::string Name = "";
        // Inlet and outlet information
        std::string InletNodeName = "";
        std::string OutletNodeName = "";
        int InletNodeNum = 0;
        int OutletNodeNum = 0;
        Point3DInteger CircuitInletCell;
        Point3DInteger CircuitOutletCell;
        // Names and pointers to pipe segments found in this pipe circuit
        Array1D_string PipeSegmentNames;
        Array1D_int PipeSegmentIndices;
        // Pointer to the domain which contains this pipe circuit
        int ParentDomainIndex = 0;
        int CircuitIndex = 0;
        // Misc inputs
        RadialSizing PipeSize;
        RadialSizing InsulationSize;
        Real64 RadialMeshThickness = 0.0;
        bool HasInsulation = false;
        Real64 DesignVolumeFlowRate = 0.0;
        Real64 DesignMassFlowRate = 0.0;
        Real64 Convergence_CurrentToPrevIteration = 0.0;
        int MaxIterationsPerTS = 0;
        int NumRadialCells = 0;
        BaseThermalPropertySet PipeProperties;
        BaseThermalPropertySet InsulationProperties;
        // A list of 3d cell indices that span the entire length of this pipe circuit (useful for reporting)
        Array1D<Point3DInteger> ListOfCircuitPoints;
        // Flags
        bool CheckEquipName = true;
        bool NeedToFindOnPlantLoop = true;
        bool IsActuallyPartOfAHorizontalTrench = false;
        // Location of this pipe circuit in the PlantLoop topology
        int LoopNum = 0;
        int LoopSideNum = 0;
        int BranchNum = 0;
        int CompNum = 0;
        ExtendedFluidProperties CurFluidPropertySet; // is_used
        // Variables used to pass information from INIT-type routines to CALC-type routines
        Real64 CurCircuitInletTemp = 23.0;
        Real64 CurCircuitFlowRate = 0.1321;
        Real64 CurCircuitConvectionCoefficient = 0.0;
        // Reporting variables
        Real64 InletTemperature = 0.0;
        Real64 OutletTemperature = 0.0;
        Real64 FluidHeatLoss = 0.0;

        // Default Constructor
        PipeCircuitInfo() = default;
        virtual ~PipeCircuitInfo() = default;

        void initInOutCells(CartesianCell const &in, CartesianCell const &out);

        static PlantComponent *factory(int objectType, std::string objectName);

        void simulate(const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

    };

    struct ZoneCoupledSurfaceData
    {
        // Members
        // ID
        std::string Name;
        // Surface data
        int IndexInSurfaceArray;
        Real64 SurfaceArea;
        Real64 Width;
        Real64 Length;
        Real64 Depth;
        Real64 Conductivity;
        Real64 Density;
        Real64 InsulationConductivity;
        Real64 InsulationDensity;
        int Zone;

        // Default Constructor
        ZoneCoupledSurfaceData()
            : IndexInSurfaceArray(0), SurfaceArea(0.0), Width(0.0), Length(0.0), Depth(0.0), Conductivity(0.0), Density(0.0),
              InsulationConductivity(0.0), InsulationDensity(0.0), Zone(0)
        {
        }
    };

    struct FullDomainStructureInfo
    {
        // Members
        // ID
        std::string Name;
        // Names and pointers to circuits found in this domain
        Array1D_string CircuitNames;
        Array1D_int CircuitIndices;
        int MaxIterationsPerTS;
        // Flag variables
        bool OneTimeInit;
        bool BeginSimInit;
        bool BeginSimEnvironment;
        bool DomainNeedsSimulation;
        bool DomainNeedsToBeMeshed;
        bool IsActuallyPartOfAHorizontalTrench;
        bool HasAPipeCircuit;
        bool HasZoneCoupledSlab;
        bool HasZoneCoupledBasement;
        // "Input" data structure variables
        MeshExtents Extents;
        MeshProperties Mesh;
        BaseThermalPropertySet GroundProperties;
        BaseThermalPropertySet SlabProperties;
        BaseThermalPropertySet BasementInterfaceProperties;
        BaseThermalPropertySet HorizInsProperties;
        BaseThermalPropertySet VertInsProperties;
        SimulationControl SimControls;
        FarfieldInfo Farfield;
        BasementZoneInfo BasementZone;
        MoistureInfo Moisture;
        // "Internal" data structure variables
        MeshPartitions Partitions;
        CurSimConditionsInfo Cur;
        bool HasBasement;
        // Zone coupled variables
        std::vector<ZoneCoupledSurfaceData> ZoneCoupledSurfaces;
        int ZoneCoupledOSCMIndex;
        Real64 PerimeterOffset;
        bool SlabInGradeFlag;
        int SlabMaterialNum;
        Real64 SlabArea;
        Real64 SlabWidth;
        Real64 SlabLength;
        Real64 SlabThickness;
        int XIndex;
        int YIndex;
        int ZIndex;
        int x_max_index;
        int y_max_index;
        int z_max_index;
        bool HorizInsPresentFlag;
        int HorizInsMaterialNum;
        Real64 HorizInsThickness;
        Real64 HorizInsWidth;
        Real64 HeatFlux;
        Real64 WallHeatFlux;
        Real64 FloorHeatFlux;
        Real64 AggregateHeatFlux;
        Real64 AggregateWallHeatFlux;
        Real64 AggregateFloorHeatFlux;
        int NumHeatFlux;
        bool ResetHeatFluxFlag;
        Real64 ConvectionCoefficient;
        bool FullHorizInsPresent;
        bool VertInsPresentFlag;
        int VertInsMaterialNum;
        Real64 VertInsThickness;
        Real64 VertInsDepth;
        int XWallIndex;
        int YFloorIndex;
        int ZWallIndex;
        int InsulationXIndex;
        int InsulationYIndex;
        int InsulationZIndex;
        bool SimTimeStepFlag;
        bool SimHourlyFlag;
        bool SimDailyFlag;
        Real64 ZoneCoupledSurfaceTemp;
        Real64 BasementWallTemp;
        Real64 BasementFloorTemp;
        int NumDomainCells;
        int NumGroundSurfCells;
        int NumInsulationCells;
        int NumSlabCells;
        Array2D<Real64> WeightingFactor;
        Array2D<Real64> WeightedHeatFlux;
        Real64 TotalEnergyUniformHeatFlux = 0.0;
        Real64 TotalEnergyWeightedHeatFlux = 0.0;
        Real64 HeatFluxWeightingFactor = 0.0;
        Array1D<GridRegion> XRegions;
        Array1D<GridRegion> YRegions;
        Array1D<GridRegion> ZRegions;

        // Main 3D cells array
        Array3D<CartesianCell> Cells;

        // Dynamic indexes to available neighbor directions for a particular cell
        std::vector<Direction> NeighborFieldCells;
        std::vector<Direction> NeighborBoundaryCells;

        // Default Constructor
        FullDomainStructureInfo()
            : MaxIterationsPerTS(10), OneTimeInit(true), BeginSimInit(true), BeginSimEnvironment(true), DomainNeedsSimulation(true),
              DomainNeedsToBeMeshed(true), IsActuallyPartOfAHorizontalTrench(false), HasAPipeCircuit(true), HasZoneCoupledSlab(false),
              HasZoneCoupledBasement(false), HasBasement(false), ZoneCoupledOSCMIndex(0), PerimeterOffset(0.0), SlabInGradeFlag(false),
              SlabMaterialNum(0), SlabArea(0.0), SlabWidth(0.0), SlabLength(0.0), SlabThickness(0.0), XIndex(0), YIndex(0), ZIndex(0), x_max_index(0),
              y_max_index(0), z_max_index(0), HorizInsPresentFlag(false), HorizInsMaterialNum(0), HorizInsThickness(0.0254), HorizInsWidth(0.0),
              HeatFlux(0.0), WallHeatFlux(0.0), FloorHeatFlux(0.0), AggregateHeatFlux(0.0), AggregateWallHeatFlux(0.0), AggregateFloorHeatFlux(0.0),
              NumHeatFlux(0), ResetHeatFluxFlag(true), ConvectionCoefficient(0.0), FullHorizInsPresent(false), VertInsPresentFlag(false), VertInsMaterialNum(0),
              VertInsThickness(0.0254), VertInsDepth(0.0), XWallIndex(0), YFloorIndex(0), ZWallIndex(0), InsulationXIndex(0), InsulationYIndex(0),
              InsulationZIndex(0), SimTimeStepFlag(false), SimHourlyFlag(false), SimDailyFlag(false), ZoneCoupledSurfaceTemp(0.0),
              BasementWallTemp(0.0), BasementFloorTemp(0.0), NumDomainCells(0), NumGroundSurfCells(0), NumInsulationCells(0), NumSlabCells(0),
              XRegions({0, -1}), YRegions({0, -1}), ZRegions({0, -1})
        {
            NeighborFieldCells.reserve(6);
            NeighborBoundaryCells.reserve(6);
        }

        void developMesh();

        void createPartitionCenterList();

        Array1D<GridRegion> createPartitionRegionList(Array1D<MeshPartition> const &ThesePartitionCenters,
                                                      bool PartitionsExist,
                                                      Real64 DirExtentMax,
                                                      int PartitionsUBound);

        void createRegionList(Array1D<GridRegion> &Regions,
                              Array1D<GridRegion> const &ThesePartitionRegions,
                              Real64 DirExtentMax,
                              RegionType DirDirection,
                              bool PartitionsExist,
                              Optional_int BasementWallXIndex = _,
                              Optional_int BasementFloorYIndex = _,
                              Optional_int XIndex = _,
                              Optional_int XWallIndex = _,
                              Optional_int InsulationXIndex = _,
                              Optional_int YIndex = _,
                              Optional_int YFloorIndex = _,
                              Optional_int InsulationYIndex = _,
                              Optional_int ZIndex = _,
                              Optional_int ZWallIndex = _,
                              Optional_int InsulationZIndex = _);

        void createCellArray(Array1D<Real64> const &XBoundaryPoints, Array1D<Real64> const &YBoundaryPoints, Array1D<Real64> const &ZBoundaryPoints);

        void setupCellNeighbors();

        void setupPipeCircuitInOutCells();

        int getCellWidthsCount(RegionType dir);

        void getCellWidths(GridRegion &g, RegionType direction);

        void addNeighborInformation(int X,
                                    int Y,
                                    int Z,
                                    Direction direction,
                                    Real64 ThisCentroidToNeighborWall,
                                    Real64 ThisWallToNeighborCentroid,
                                    Real64 ThisAdiabaticMultiplier);

        Real64 GetBasementWallHeatFlux();

        Real64 GetBasementFloorHeatFlux();

        void UpdateBasementSurfaceTemperatures();

        Real64 GetZoneInterfaceHeatFlux();

        void UpdateZoneSurfaceTemperatures();

        Real64 GetAverageTempByType(CellType cellType);

        void InitializeSoilMoistureCalcs();

        void EvaluateSoilRhoCp(Real64 CellTemp, Real64 &rhoCp);

        void ShiftTemperaturesForNewTimeStep();

        void ShiftTemperaturesForNewIteration();

        bool IsConverged_CurrentToPrevIteration();

        bool CheckForOutOfRangeTemps();

        void EvaluateNeighborCharacteristics(CartesianCell &ThisCell,
                                             Direction CurDirection,
                                             Real64 &NeighborTemp,
                                             Real64 &Resistance,
                                             Real64 &AdiabaticMultiplier);

        void EvaluateCellNeighborDirections(CartesianCell const &cell, int &NumFieldCells, int &NumBoundaryCells);

        void DoEndOfIterationOperations(bool &Finished);

        void DoOneTimeInitializations(Optional<int const> CircuitNum);

        void DoStartOfTimeStepInitializations(Optional<int const> CircuitNum);

        Real64 GetFarfieldTemp(CartesianCell const &cell);

        void PreparePipeCircuitSimulation(int CircuitNum);

        void PerformPipeCircuitSimulation(int CircuitNum);

        void PerformPipeCellSimulation(int CircuitNum, CartesianCell &ThisCell, Real64 FlowRate, Real64 EnteringTemp);

        void SimulateRadialToCartesianInterface(CartesianCell &ThisCell);

        void PerformTemperatureFieldUpdate();

        Real64 EvaluateFieldCellTemperature(CartesianCell &ThisCell);

        Real64 EvaluateGroundSurfaceTemperature(CartesianCell &cell);

        Real64 EvaluateBasementCellTemperature(CartesianCell &cell);

        Real64 EvaluateZoneInterfaceTemperature(CartesianCell &cell);

        Real64 EvaluateFarfieldBoundaryTemperature(CartesianCell &cell);

        void EvaluateFarfieldCharacteristics(CartesianCell &cell, Direction direction, Real64 &neighbortemp, Real64 &resistance, Real64 &adiabaticMultiplier);

        void PerformIterationLoop(Optional<int const> CircuitNum);

        void InitPipingSystems(int CircuitNum);

        void UpdatePipingSystems(int CircuitNum);

        void SetupZoneCoupledOutputVariables();

    };

    // Object Data
    extern Array1D<FullDomainStructureInfo> PipingSystemDomains;
    extern Array1D<PipeCircuitInfo> PipingSystemCircuits;
    extern Array1D<PipeSegmentInfo> PipingSystemSegments;

    void clear_state();

    void SimulateGroundDomains(bool initOnly);

    void CheckIfAnySlabs();

    void CheckIfAnyBasements();

    void GetPipingSystemsAndGroundDomainsInput();

    int GetNumSegmentsForHorizontalTrenches(int NumHorizontalTrenches);

    void ReadGeneralDomainInputs(int IndexStart, int NumGeneralizedDomains, bool &ErrorsFound);

    void ReadZoneCoupledDomainInputs(int StartingDomainNumForZone, int NumZoneCoupledDomains, bool &ErrorsFound);

    void ReadBasementInputs(int StartingDomainNumForBasement, int NumBasements, bool &ErrorsFound);

    void ReadPipeCircuitInputs(int NumPipeCircuits, bool &ErrorsFound);

    void ReadPipeSegmentInputs(int NumPipeSegmentsInInput, bool &ErrorsFound);

    void ReadHorizontalTrenchInputs(int StartingDomainNumForHorizontal,
                                    int StartingCircuitNumForHorizontal,
                                    int StartingSegmentNumForHorizontal,
                                    int NumHorizontalTrenchesInInput,
                                    bool &ErrorsFound);

    void SetupPipingSystemOutputVariables(int TotalNumSegments, int TotalNumCircuits);

    void IssueSevereInputFieldErrorStringEntry(std::string const &RoutineName,
                                               std::string const &ObjectName,
                                               std::string const &InstanceName,
                                               std::string const &FieldName,
                                               std::string const &FieldEntry,
                                               std::string const &Condition,
                                               bool &ErrorsFound);

    void IssueSevereInputFieldErrorRealEntry(std::string const &RoutineName,
                                             std::string const &ObjectName,
                                             std::string const &InstanceName,
                                             std::string const &FieldName,
                                             Real64 FieldEntry,
                                             std::string const &Condition,
                                             bool &ErrorsFound);

    int GetSurfaceCountForOSCM(int OSCMIndex);

    std::vector<int> GetSurfaceIndecesForOSCM(int OSCMIndex);

    std::vector<ZoneCoupledSurfaceData> GetSurfaceDataForOSCM(int OSCMIndex);

    bool inline IsInRangeReal(Real64 const r, Real64 const lower, Real64 const upper)
    {
        return ((r >= lower) && (r <= upper));
    }

    bool inline IsInRange_BasementModel(Real64 const r, Real64 const lower, Real64 const upper)
    {
        return ((r >= lower) && (r < upper));
    }

    bool MeshPartitionArray_Contains(Array1D<MeshPartition> const &meshes, Real64 value);

    void MeshPartition_SelectionSort(Array1<MeshPartition> &X);

    void ShiftPipeTemperaturesForNewIteration(CartesianCell &ThisPipeCell);

    int CreateBoundaryListCount(Array1D<GridRegion> const &RegionList, RegionType DirDirection);

    Array1D<Real64> CreateBoundaryList(Array1D<GridRegion> const &RegionList,
                                       Real64 DirExtentMax,
                                       RegionType DirDirection,
                                       int RetValLbound,
                                       int RetValUBound);

    void SimulateOuterMostRadialSoilSlice(int CircuitNum, CartesianCell &ThisCell);

    void SimulateAllInteriorRadialSoilSlices(CartesianCell &ThisCell);

    void SimulateInnerMostRadialSoilSlice(int CircuitNum, CartesianCell &ThisCell);

    void SimulateRadialInsulationCell(CartesianCell &ThisCell);

    void SimulateRadialPipeCell(int CircuitNum, CartesianCell &ThisCell, Real64 ConvectionCoefficient);

    void SimulateFluidCell(CartesianCell &ThisCell, Real64 FlowRate, Real64 ConvectionCoefficient, Real64 EnteringFluidTemp);

    bool IsConverged_PipeCurrentToPrevIteration(int CircuitNum, CartesianCell const &CellToCheck);

} // namespace PlantPipingSystemsManager

} // namespace EnergyPlus

#endif
