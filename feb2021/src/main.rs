extern crate nalgebra as na;
extern crate image;

use na::{Vector3, Vector2};
use image::{GenericImage, DynamicImage, Rgba};
use std::time::{Instant};
use std::thread;

const WIDTH: u32 = 800;
const HEIGHT: u32 = 600;
const FOV: f32 = 90.0;
const AS_R: f32 = WIDTH as f32 / HEIGHT as f32;
const GAMMA: f32 = 2.2;

const MAX_STEPS: u32 = 100;
const SURF_DIST: f32 = 0.01;
const MAX_DIST : f32 = 100.0;

fn main() {
    let mut img: DynamicImage = DynamicImage::new_rgba8(WIDTH, HEIGHT);
    let light_pos = Vector3::new(0.0, 0.0, 0.0);

    let exec_time = Instant::now();
    for y in 0..HEIGHT {
        for x in 0..WIDTH {
            let ray = Ray::new(x as f32, y as f32);
            let mut col = Vector3::new(0.0, 0.0, 0.0);

            let hit = march(&ray);
            if hit < MAX_DIST {
                // col = Vector3::new(hit, hit, hit)/8.0;
                let p = ray.o + ray.d * hit;
                let light_d = -(p - light_pos).normalize();
                let normal = estimate_normal(&p);
                let diff_ints = normal.dot(&light_d).max(0.0);
                col = Vector3::new(1.0, 0.0, 0.0) * diff_ints;
            }
            img.put_pixel(x, y, to_rgba(&col));
        }
    }
    let exec_dur = exec_time.elapsed();

    img.save("out.png").unwrap();
    println!("Time rendering: {:?}", exec_dur);
}

fn march(r: &Ray) -> f32 {
    let mut d_o: f32 = 0.0;

    for _ in 0..MAX_STEPS {
        let p: Vector3<f32> = r.o + r.d * d_o;
        let d_s = scene_sdf(&p);
        d_o += d_s;
        if d_s < SURF_DIST || d_o > MAX_DIST {
            break;
        }
    }

    d_o
}

fn estimate_normal(p: &Vector3<f32>) -> Vector3<f32> {
    return Vector3::new(
        scene_sdf(&Vector3::new(p.x + SURF_DIST, p.y, p.z)) - scene_sdf(&Vector3::new(p.x - SURF_DIST, p.y, p.z)),
        scene_sdf(&Vector3::new(p.x, p.y + SURF_DIST, p.z)) - scene_sdf(&Vector3::new(p.x, p.y - SURF_DIST, p.z)),
        scene_sdf(&Vector3::new(p.x, p.y, p.z + SURF_DIST)) - scene_sdf(&Vector3::new(p.x, p.y, p.z - SURF_DIST))
    ).normalize();
}

fn scene_sdf(p: &Vector3<f32>) -> f32 {
    let b1 = Vector3::new(0.1, 0.1, 0.1);

    let q1: Vector3<f32> = (p - Vector3::new(1.0, 0.25, -1.0)).abs() - b1;
    let d1 = q1.sup(&Vector3::new(0.0, 0.0, 0.0)).magnitude();
    
    let b2 = Vector3::new(0.25, 0.1, 0.1);

    let q2: Vector3<f32> = (p - Vector3::new(0.5, 0.0, -1.0)).abs() - b2;
    let d2 = q2.sup(&Vector3::new(0.0, 0.0, 0.0)).magnitude();

    let cyl_p: Vector3<f32> = p - Vector3::new(0.0, 0.0, -1.0);
    let q3: Vector2<f32> = Vector2::new(Vector2::new(cyl_p.x, cyl_p.z).magnitude(), cyl_p.y).abs() - Vector2::new(1.0, 0.25);
    let d3 = min(max(q3.x, q3.y), 0.0) + q3.sup(&(Vector2::new(0.0, 0.0))).magnitude();

    min(min(d1, d2), d3)
}

struct Ray {
    o: Vector3<f32>,
    d: Vector3<f32>
}

impl Ray {
    fn new(x: f32, y: f32) -> Self {
        let fov: f32 = (FOV.to_radians() / 2.0).tan();
        let r_x: f32 = (((x + 0.5) / WIDTH as f32) * 2.0 - 1.0) * AS_R * fov;
        let r_y: f32 = (1.0 - ((y + 0.5) / HEIGHT as f32) * 2.0) * fov;

        Ray {
            o: Vector3::new(0.0, 0.0, 0.0),
            d: Vector3::new(r_x, r_y, -1.0).normalize()
        }
    }
}

fn to_rgba(c: &Vector3<f32>) -> Rgba<u8> {
    Rgba([
        (gamma_encode(c.x) * 255.0) as u8,
        (gamma_encode(c.y) * 255.0) as u8,
        (gamma_encode(c.z) * 255.0) as u8,
        255
    ])
}

fn gamma_encode(l: f32) -> f32 {
    l.powf(1.0/GAMMA)
}

fn min(a: f32, b: f32) -> f32 {
    return if a > b { b } else { a };
}

fn max(a: f32, b: f32) -> f32 {
    return if a < b { b } else { a };
}
