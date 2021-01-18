extern crate nalgebra as na;
extern crate image as image;
use na::{Vector3};
use image::{DynamicImage, Rgba};
use crate::image::GenericImage;
use std::ops::{Add, Mul, Div};
use std::time::{Instant};
use rand::Rng;

const WIDTH: u32 = 800;
const HEIGHT: u32 = 600;
const ASR: f32 = WIDTH as f32 / HEIGHT as f32;
const FOV: f32 = 90.0;
const GAMMA: f32 = 2.2;
const SS: u8 = 1;

fn main() {
    let mut world_spheres: Vec<Sphere> = Vec::new();
    world_spheres.push(Sphere::new(-1.0, 0.0, -5.0, 3.0, Color::new(200.0/255.0, 35.0/255.0, 35.0/255.0)));
    world_spheres.push(Sphere::new(2.2, 1.0, -3.0, 1.0, Color::new(35.0/255.0, 200.0/255.0, 35.0/255.0)));
    world_spheres.push(Sphere::new(-3.0, -1.0, -2.8, 0.5, Color::new(35.0/255.0, 35.0/255.0, 200.0/255.0)));
    let light = Light::new(Vector3::new(2.0, 1.0, -1.0), 5.0);
    let mut image = DynamicImage::new_rgba8(WIDTH, HEIGHT);
    // let background = Color::new(173.0/255.0, 216.0/255.0, 230.0/255.0);
    // let background = Color::new(0.1, 0.1, 0.1);
    let bg_img = image::open("src/bg.jpg").unwrap().to_rgba();

    let mut rng = rand::thread_rng();
    let exec_time = Instant::now();
    for x in 0..WIDTH {
        for y in 0..HEIGHT {
            let mut out_col = Color::new(0.0, 0.0, 0.0);
            for _ in 0..SS {
                // let ray = new_ray(x as f32 + rng.gen_range(0.0, 1.0), y as f32 + rng.gen_range(0.0, 1.0));
                let ray = new_ray(x as f32, y as f32);

                let (sp_i, hit_t) = trace_inersection(&ray, world_spheres.as_slice());
                let sphere = &world_spheres[sp_i];
                
                if hit_t != std::f32::MAX {
                    let hit_p = ray.o + ray.d * hit_t;
                    let norm = sphere.sphere_normal(&hit_p);
                    let light_v = light.o - hit_p;
                    let light_n = light_v.normalize();
                    if !one_time_intersection(&Ray {o: hit_p + norm * 1e-4, d: light_n}, world_spheres.as_slice()) {
                        let dist: f32 = light_v.norm() as f32;
                        let lint = norm.dot(&light_n) * light.i / dist.powf(2.0);
                        out_col = out_col + (Color::from_color(&sphere.col) * lint);
                    }
                } else {
                    out_col = out_col + Color::from_rgba(*bg_img.get_pixel(x, y)); //Color::from_color(&background);
                }
            }
            out_col = out_col / (SS as f32);
            image.put_pixel(x, y, out_col.to_rgba());
        }
    }
    let exec_dur = exec_time.elapsed();

    image.save("img.png").unwrap();
    println!("Time rendering: {:?}", exec_dur);
}

fn new_ray(x: f32, y: f32) -> Ray {
    let fov = (FOV.to_radians() / 2.0).tan();
    let r_x: f32 = (((x + 0.5) / WIDTH as f32) * 2.0 - 1.0) * ASR * fov;
    let r_y: f32 = (1.0 - ((y + 0.5) / HEIGHT as f32) * 2.0) * fov;

    Ray {
        o: Vector3::new(0.0, 0.0, 0.0),
        d: Vector3::new(r_x, r_y, -1.0).normalize()
    }
}

fn trace_inersection(ray: &Ray, spheres: &[Sphere]) -> (usize, f32) {
    let mut sp_i = 0;
    let mut hit_t = std::f32::MAX;

    for i in 0..spheres.len() {
        let t = sphere_intersection(&spheres[i], &ray);
        if t < hit_t {
            hit_t = t;
            sp_i = i;
        }
    }

    (sp_i, hit_t)
}

fn one_time_intersection(ray: &Ray, spheres: &[Sphere]) -> bool {
    for s in spheres {
        if sphere_intersection(&s, &ray) != std::f32::MAX {
            return true;
        }
    }
    
    false
}

// fn reflc_ray(ray: &Ray, hit_p: &Vector3<f32>, norm: &Vector3<f32>) -> Ray {
//     Ray {
//         o: *hit_p,
//         d: reflection(&ray.d, norm)
//     }
// }

// fn reflection(dir: &Vector3<f32>, norm: &Vector3<f32>) -> Vector3<f32> {
//     2.0 * (norm.dot(dir)) * (norm - dir)
// }

fn sphere_intersection(sp: &Sphere, ry: &Ray) -> f32 {
    let hyp = sp.o - ry.o;
    let t_h: f32 = hyp.dot(&ry.d);
    let a2 = hyp.dot(&hyp) - t_h*t_h;

    if a2 > sp.r2 {
        return std::f32::MAX
    }

    let thc = (sp.r2 - a2).sqrt();
    let mut t0 = t_h - thc;
    let mut t1 = t_h + thc;

    if t0 > t1 {
        std::mem::swap(&mut t0, &mut t1);
    }

    if t0 < 0.0 {
        t0 = t1;
        if t0 < 0.0 {
            return std::f32::MAX
        }
    }

    return t0
}

fn plane_intersection(pl: &Plane, ry: &Ray) -> f32 {
    let denom = pl.n.dot(&ry.d);
    if denom > 1e-6 {
        let p0l0 = pl.o - ry.o;
        let t: f32 = p0l0.dot(&pl.n) / denom;
        if t >= 0.0 {
            return t;
        }
    }
    std::f32::MAX
}

struct Ray {
    o: Vector3<f32>,
    d: Vector3<f32>
}

struct Sphere {
    o: Vector3<f32>,
    r: f32,
    r2: f32,
    col: Color
}

impl Sphere {
    pub fn new(x: f32, y: f32, z: f32, r: f32, c: Color) -> Self {
        Sphere {
            o: Vector3::new(x, y, z),
            r: r,
            r2: r*r,
            col: c
        }
    }

    pub fn sphere_normal(&self , hit_p: &Vector3<f32>) -> Vector3<f32> {
        (hit_p - self.o).normalize()
    }
}

struct Plane {
    o: Vector3<f32>,
    n: Vector3<f32>
}

impl Plane {
    pub fn new(x: f32, y: f32, z: f32, n: Vector3<f32>, c: Color) -> Self {
        Plane {
            o: Vector3::new(x, y, z),
            n: n
        }
    }

    pub fn plane_normal(&self) -> Vector3<f32> {
        self.n
    }
}

struct Light {
    o: Vector3<f32>,
    i: f32,
}

impl Light {
    pub fn new(o: Vector3<f32>, i: f32) -> Self {
        Light {
            o: o,
            i: i,
        }
    }
}

struct Color {
    r: f32,
    g: f32,
    b: f32
}

fn gamma_encode(linear: f32) -> f32 {
    linear.powf(1.0/GAMMA)
}

fn gamma_decode(encoded: f32) -> f32 {
    encoded.powf(GAMMA)
}

impl Color {
    pub fn new(r: f32, g: f32, b:f32) -> Self {
        Color {
            r: r,
            g: g,
            b: b
        }
    }

    pub fn to_rgba(&self) -> Rgba<u8> {
        Rgba([
            (gamma_encode(self.r) * 255.0) as u8,
            (gamma_encode(self.g) * 255.0) as u8,
            (gamma_encode(self.b) * 255.0) as u8,
            255
        ])
    }

    pub fn from_rgba(rgba: Rgba<u8>) -> Color {
        Color {
            r: gamma_decode((rgba.0[0] as f32) / 255.0),
            g: gamma_decode((rgba.0[1] as f32) / 255.0),
            b: gamma_decode((rgba.0[2] as f32) / 255.0),
        }
    }

    pub fn from_color(c: &Color) -> Color {
        Color {
            r: c.r,
            g: c.g,
            b: c.b
        }
    }
}

impl Mul<f32> for Color {
    type Output = Color;

    fn mul(mut self, n: f32) -> Color {
        self.r *= n;
        self.g *= n;
        self.b *= n;

        return self;
    }
}

impl Mul<Color> for Color {
    type Output = Color;

    fn mul(mut self, c: Color) -> Color {
        self.r *= c.r;
        self.g *= c.g;
        self.b *= c.b;

        return self;
    }
}

impl Add<Color> for Color {
    type Output = Color;

    fn add(mut self, c: Color) -> Color {
        self.r += c.r;
        self.g += c.g;
        self.b += c.b;

        return self;
    }
}

impl Div<f32> for Color {
    type Output = Color;

    fn div(mut self, n: f32) -> Color {
        self.r /= n;
        self.g /= n;
        self.b /= n;

        return self;
    }
}

impl Div<Color> for Color {
    type Output = Color;

    fn div(mut self, c: Color) -> Color {
        self.r /= c.r;
        self.g /= c.g;
        self.b /= c.b;

        return self;
    }
}

