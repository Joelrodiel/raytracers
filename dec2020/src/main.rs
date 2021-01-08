extern crate nalgebra as na;
extern crate image;
extern crate regex;
use na::{Vector3};
use image::{DynamicImage, Rgba, GenericImage};
use std::time::Instant;
use std::fs::File;
use std::io::{BufRead, BufReader};
use regex::Regex;

const WIDTH: u32 = 800;
const HEIGHT: u32 = 600;
const ASR: f32 = WIDTH as f32 / HEIGHT as f32;
const FOV: f32 = 90.0;

fn main() {
    let mut triangles: Vec<Triangle> = Vec::new();
    let mut verts: Vec<Vector3<f32>> = Vec::new();

    let file = File::open("teapot.obj").expect("Unable to open file.");
    let bfr  = BufReader::new(file);
    let reg1 = Regex::new(r"^v\s(-?\d+.\d+)\s(-?\d+.\d+)\s(-?\d+.\d+)$").unwrap();
    let reg2 = Regex::new(r"^f\s(\d+)\s(\d+)\s(\d+)$").unwrap();

    let r_middl = new_ray(WIDTH/2, HEIGHT/2).d;

    for v in bfr.lines() {
        let line = v.expect("Unable to read line");
        if reg1.is_match(&line) {
            let caps = reg1.captures(&line).unwrap();
            let vec1 = caps.get(1).map_or("", |m| m.as_str()).parse::<f32>().unwrap();
            let vec2 = caps.get(2).map_or("", |m| m.as_str()).parse::<f32>().unwrap();
            let vec3 = caps.get(3).map_or("", |m| m.as_str()).parse::<f32>().unwrap();
            verts.push(Vector3::new(vec1, vec2, vec3));
        } else if reg2.is_match(&line) {
            let caps = reg2.captures(&line).unwrap();
            let vec1 = caps.get(1).map_or("", |m| m.as_str()).parse::<usize>().unwrap();
            let vec2 = caps.get(2).map_or("", |m| m.as_str()).parse::<usize>().unwrap();
            let vec3 = caps.get(3).map_or("", |m| m.as_str()).parse::<usize>().unwrap();
            let tri = Triangle::new(verts[vec1 - 1], verts[vec2 - 1], verts[vec3 - 1]);
            
            if tri.n.dot(&r_middl) < 0.0 {
                triangles.push(tri);
            }
        }
    }

    let mut img = DynamicImage::new_rgba8(WIDTH, HEIGHT);
    let exec_t  = Instant::now();

    for x in 0..WIDTH {
        for y in 0..HEIGHT {
            let ray = new_ray(x, y);
            let mut out_color = Rgba([0, 0, 0, 255]);
            let mut smallest  = std::f32::MAX;
            let mut index: usize = 0;

            for i in 0..triangles.len() {
                let t = triangle_intersect_mt(&triangles[i], &ray);
                if t < smallest {
                    smallest = t;
                    index = i;
                }
            }

            let light = Vector3::new(0.0, 0.0, 1.0).normalize();

            if smallest < std::f32::MAX {
                let t_norm = triangles[index].n.normalize();
                let shade = light.dot(&t_norm).abs() + 0.1;
                let sh_cl = (255.0*shade) as u8;
                out_color = Rgba([sh_cl, sh_cl, sh_cl, 255]);
                // out_color = Rgba([(255.0*t_norm.x).abs() as u8, (255.0*t_norm.y).abs() as u8, (255.0*t_norm.z).abs() as u8, 255])
            }

            img.put_pixel(x, y, out_color);
        }
    }

    let exec_d  = exec_t.elapsed();
    img.save("img.png").unwrap();
    println!("Time rendering: {:?}", exec_d);
}

struct Ray {
    o: Vector3<f32>,
    d: Vector3<f32>
}

fn new_ray(x: u32, y: u32) -> Ray {
    let fov: f32 = (FOV.to_radians() / 2.0).tan();
    let r_x: f32 = (((x as f32 + 0.5) / WIDTH as f32) * 2.0 - 1.0) * ASR * fov;
    let r_y: f32 = ((1.0 - (y as f32 + 0.5) / HEIGHT as f32) * 2.0) * fov;

    Ray {
        o: Vector3::new(0.0, 0.0, 0.0),
        d: Vector3::new(r_x, r_y, -1.0)
    }
}

struct Triangle {
    o0: Vector3<f32>,
    o1: Vector3<f32>,
    o2: Vector3<f32>,
    n: Vector3<f32>,
    d: f32
}

impl Triangle {
    pub fn new(o0: Vector3<f32>, o1: Vector3<f32>, o2: Vector3<f32>) -> Self {
        let norm = (o1 - o0).cross(&(o2 - o0));
        Triangle {
            o0: o0,
            o1: o1,
            o2: o2,
            n: norm,
            d: norm.dot(&o0)
        }
    }
}

fn triangle_intersect(tri: &Triangle, r: &Ray) -> f32 {
    let paralel = tri.n.dot(&r.d);
    if paralel > 0.0 {
        return std::f32::MAX;
    }

    let t = (tri.n.dot(&r.o) + tri.d) / paralel;
    if t < 0.0 {
        return std::f32::MAX;
    }

    let p = r.o + r.d * t;

    let edge_0 = tri.o1 - tri.o0;
    let op_0 = p - tri.o0;
    let mut c = edge_0.cross(&op_0);
    if tri.n.dot(&c) < 0.0 {
        return std::f32::MAX;
    }

    let edge_1 = tri.o2 - tri.o1;
    let op_1 = p - tri.o1;
    c = edge_1.cross(&op_1);
    if tri.n.dot(&c) < 0.0 {
        return std::f32::MAX;
    }

    let edge_2 = tri.o0 - tri.o2;
    let op_2 = p - tri.o2;
    c = edge_2.cross(&op_2);
    if tri.n.dot(&c) < 0.0 {
        return std::f32::MAX;
    }

    t
}

fn triangle_intersect_mt(tri: &Triangle, ray: &Ray) -> f32 {
    let v0_v1 = tri.o1 - tri.o0;
    let v0_v2 = tri.o2 - tri.o0;
    let p_v   = ray.d.cross(&v0_v2);
    let det   = v0_v1.dot(&p_v);

    if det <= 0.0 {
        return std::f32::MAX;
    }

    let inv_det = 1.0 / det;

    let t_vec = ray.o - tri.o0;
    let u     = t_vec.dot(&p_v) * inv_det;
    if u < 0.0 || u > 1.0 {
        return std::f32::MAX;
    }

    let q_v = t_vec.cross(&v0_v1);
    let v   = ray.d.dot(&q_v) * inv_det;
    if v < 0.0 || u + v > 1.0 {
        return std::f32::MAX;
    }

    v0_v2.dot(&q_v) * inv_det
}
