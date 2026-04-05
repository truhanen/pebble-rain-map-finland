import dataclasses
import pickle
from pathlib import Path
from typing import Callable

import geopandas as gpd
import matplotlib.pyplot as plt
import rasterio
import requests
from rasterio import features
from shapely.affinity import translate, scale
from shapely.geometry import box, MultiLineString, LineString, Point, GeometryCollection
from shapely.ops import linemerge, unary_union, split, snap, voronoi_diagram
from shapely import set_precision
from skan import Skeleton
from skimage import morphology
from svgwrite.drawing import Drawing

# Script https://github.com/pebble-examples/cards-example/blob/master/tools/svg2pdc.py
# Note that the script must be slightly modified to work on new Python & library
# versions
from svg2pdc import create_pdc_from_path

CACHE_DIR = Path(__file__).parent / "cache"

# File downloaded from https://osmdata.openstreetmap.de/data/coastlines.html
COASTLINE_PATH = CACHE_DIR / "coastlines-split-4326" / "lines.shp"

OVERPASS_URL = "https://overpass-api.de/api/interpreter"

SIMPLIFICATION_TOLERANCE = 0.05

# These must be the same as the respective constants in draw_map.c
RESOURCE_LONGITUDE_MIN = 19.0
RESOURCE_LATITUDE_MIN = 57.0


@dataclasses.dataclass
class MapData:
    border: LineString
    coastlines: MultiLineString
    towns: list[Point]
    highways: MultiLineString


@dataclasses.dataclass
class CoordinateBounds:
    longitude_min: float
    latitude_min: float
    longitude_max: float
    latitude_max: float


def get_coastlines() -> MultiLineString:
    gdf = gpd.read_file(COASTLINE_PATH)

    # Bounding box with needed Baltic sea coastlines
    coastlines = gdf.clip(box(19.0, 57.0, 31.5, 66.0))

    coastlines = coastlines.geometry.union_all()

    coastlines = linemerge(coastlines)

    # Drop smaller islands. In WGS84, 0.1 is roughly 11 km. Could project the
    # values for more accuracy.
    min_perimeter = 1.5
    filtered_lines = [line for line in coastlines.geoms if line.length > min_perimeter]

    coastlines = MultiLineString(filtered_lines)

    return coastlines


def get_border() -> LineString:
    query = """
    [out:json][timeout:30];
    rel["boundary"="administrative"]["admin_level"="2"]["ISO3166-1"="FI"];
    out geom;
    """

    response = requests.get(OVERPASS_URL, params={"data": query}, timeout=30)
    response.raise_for_status()
    data = response.json()

    lines = []
    for rel in data["elements"]:
        for member in rel.get("members", []):
            if "geometry" in member:
                coordinates = [(pt["lon"], pt["lat"]) for pt in member["geometry"]]
                if len(coordinates) >= 2:
                    lines.append(LineString(coordinates))

    border = unary_union(lines)
    border = linemerge(border)

    return border


def get_towns() -> list[Point]:
    # Get also towns with `node["place"~"^(city)|town$"]`
    overpass_query = """
    [out:json][timeout:30];
    area["name:en"="Finland"]->.searchArea;
    (
      node["place"~"^(city)$"](area.searchArea);
    );
    out body;
    """

    response = requests.get(
        OVERPASS_URL,
        params={"data": overpass_query},
        timeout=30,
    )
    response.raise_for_status()
    data = response.json()

    points = []
    for element in data.get("elements", []):
        points.append(Point(element.get("lon"), element.get("lat")))
        # TODO Use "place" tag ("town" or "city")
        # tags = element.get("tags", {})
        # place_type = tags.get("place"),

    return points


def get_highways() -> MultiLineString:
    query = f"""
    [out:json][timeout:60];
    area["name:en"="Finland"]->.searchArea;
    (
      way["highway"~"^(motorway|trunk)$"](area.searchArea);
    );
    out geom;
    """

    print("Querying highways")

    response = requests.post(OVERPASS_URL, data={"data": query}, timeout=60)
    response.raise_for_status()
    data = response.json()

    lines = []
    for element in data.get("elements", []):
        if "geometry" in element:
            coords = [(node["lon"], node["lat"]) for node in element["geometry"]]
            if len(coords) >= 2:
                lines.append(LineString(coords))

    highways = MultiLineString(lines)

    return highways


def get_cached_result(path: Path, getter_function: Callable, overwrite: bool = False):
    if path.exists() and not overwrite:
        result = pickle.loads(path.read_bytes())
    else:
        result = getter_function()
        path.write_bytes(pickle.dumps(result))
    return result


def process_highways(highways: MultiLineString) -> MultiLineString:
    # Remove duplicate points
    highways = unary_union(highways)

    # Merge connected lines
    highways = linemerge(highways)

    # Some highway objects run close to each other for some distances. To join
    # them,
    # 1. buffer all lines into Polygons,
    # 2. convert to raster array,
    # 3. skeletonize, and
    # 4. convert back to MultiLineString

    # Buffer lines to polygons
    join_distance = 0.001  # ~50 m (0.1 ~ 11 km)
    highways_polygon = highways.buffer(join_distance)
    highways_polygon = unary_union(highways_polygon)

    # Rasterize
    xmin, ymin, xmax, ymax = highways_polygon.bounds
    raster_width = round((xmax - xmin) * 1000)
    raster_height = round((ymax - ymin) * 1000)
    transform = rasterio.transform.from_bounds(
        xmin, ymin, xmax, ymax, raster_width, raster_height
    )
    binary_raster = features.rasterize(
        # (geometry, fill_value) pairs
        shapes=[(geom, 1) for geom in highways_polygon.geoms],
        out_shape=(raster_height, raster_width),
        transform=transform,
        fill=0,
        dtype="uint8",
    )

    # Skeletonize
    skeleton_raster = morphology.skeletonize(binary_raster)
    skeleton = Skeleton(skeleton_raster)

    # Convert back to MultiLineString
    lines = []
    for path_index in range(skeleton.n_paths):
        skeleton_coords = skeleton.path_coordinates(path_index)
        # Flip (row, col) to (x, y) for shapely geometry
        shapely_coords = skeleton_coords[:, ::-1].astype(float)
        # Transform to (longitude, latitude)
        shapely_coords[:, 0] = xmin + shapely_coords[:, 0] / 1000
        shapely_coords[:, 1] = ymax - shapely_coords[:, 1] / 1000
        if len(shapely_coords) >= 2:
            lines.append(LineString(shapely_coords))
    highways = MultiLineString(lines)

    plot_multi_line_string(highways)

    return highways


def get_map_data() -> MapData:
    coastlines_path = CACHE_DIR / "coastlines.pkl"
    border_line_path = CACHE_DIR / "border_lines.pkl"
    towns_path = CACHE_DIR / "towns.pkl"
    highways_path = CACHE_DIR / "highways.pkl"

    CACHE_DIR.mkdir(exist_ok=True)

    coastlines = get_cached_result(coastlines_path, get_coastlines)
    border_line = get_cached_result(border_line_path, get_border)
    towns = get_cached_result(towns_path, get_towns)
    highways = get_cached_result(highways_path, get_highways)

    highways = process_highways(highways)

    main_coastline = coastlines.geoms[0]

    inland_border = split(border_line, main_coastline).geoms[1]

    inland_border = inland_border.simplify(tolerance=SIMPLIFICATION_TOLERANCE)
    coastlines = coastlines.simplify(tolerance=SIMPLIFICATION_TOLERANCE)
    highways = highways.simplify(tolerance=0.02)

    map_data = MapData(
        border=inland_border,
        coastlines=coastlines,
        towns=towns,
        highways=highways,
    )

    return map_data


def plot_multi_line_string(mls: MultiLineString):
    fig, ax = plt.subplots(figsize=(6, 8), layout="constrained")
    for geom in mls.geoms:
        ax.plot(*geom.xy, linewidth=0.7)
    plt.show()


def get_common_resource_coordinate_bounds(
    map_data: MapData,
) -> CoordinateBounds:
    """Get longitude_min, latitude_min, longitude_max, & latitude_max"""
    geoms = GeometryCollection(
        geoms=[
            map_data.border,
            *map_data.coastlines.geoms,
            *map_data.towns,
            *map_data.highways.geoms,
        ]
    )
    # Max values are based on the shapes. Min values are global constants.
    _, _, longitude_max, latitude_max = geoms.bounds

    return CoordinateBounds(
        RESOURCE_LONGITUDE_MIN,
        RESOURCE_LATITUDE_MIN,
        longitude_max,
        latitude_max,
    )


def save_transformed_svg_and_pdc(
    geoms: GeometryCollection,
    coordinate_bounds: CoordinateBounds,
    name: str,
):
    transformed = translate(
        geoms,
        xoff=-coordinate_bounds.longitude_min,
        yoff=-coordinate_bounds.latitude_min,
    )
    transformed = scale(
        transformed,
        xfact=1000.0,
        # Using positive yfact leads to the map being upside down in the svg,
        # because the svg coordinate system's origin is at the top-left corner.
        # But we need the y-coordinate to equal latitude, so this how it must
        # be.
        yfact=1000.0,
        origin=(0, 0),
    )
    transformed = set_precision(transformed, grid_size=1.0)

    xmin, ymin, xmax, ymax = transformed.bounds
    width = round(xmax - xmin)
    height = round(ymax - ymin)

    svg_path = CACHE_DIR / f"{name}.svg"

    drawing = Drawing(
        str(svg_path),
        profile="tiny",
        size=(width, height),
        viewBox=f"0 0 {width} {height}",
        preserveAspectRatio="xMinYMin",
    )
    for geom in transformed.geoms:
        if isinstance(geom, LineString):
            points = list(geom.coords)
            drawing.add(
                drawing.polyline(
                    points,
                    stroke="black",
                    fill="none",
                    stroke_width=1,
                )
            )
        elif isinstance(geom, Point):
            drawing.add(drawing.circle(tuple(*geom.coords)))
    drawing.save()

    pdc_path = (
        Path(__file__).parents[1] / "resources" / svg_path.with_suffix(".pdc").name
    )
    create_pdc_from_path(
        str(svg_path),
        sequence=False,
        out_path=pdc_path,
        verbose=True,
        duration=None,
        play_count=None,
    )


if __name__ == "__main__":
    map_data = get_map_data()

    coordinate_bounds = get_common_resource_coordinate_bounds(map_data=map_data)

    save_transformed_svg_and_pdc(
        GeometryCollection(map_data.highways.geoms),
        coordinate_bounds=coordinate_bounds,
        name="map_highways",
    )

    save_transformed_svg_and_pdc(
        GeometryCollection(
            [
                map_data.border,
                *map_data.coastlines.geoms,
            ]
        ),
        coordinate_bounds=coordinate_bounds,
        name="map_border_coastlines",
    )

    save_transformed_svg_and_pdc(
        GeometryCollection(map_data.towns),
        coordinate_bounds=coordinate_bounds,
        name="map_towns",
    )

    # plot_multi_line_string(mls=all_lines)
