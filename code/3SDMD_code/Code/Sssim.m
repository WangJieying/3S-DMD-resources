function [mssim, mcs] = Sssim(img1, img2, map, l, K, win)

if (~exist('K'))
   K = [0.01 0.03];
end

if (~exist('win'))
   win = fspecial('gaussian', 11, 1.5);
end

%img1 = imread(img1);
%img2 = imread(img2);
%map = im2double(imread(map));
map = im2double(map);
win2 = zeros(11);
win2(6,6) = 1;

map_win = filter2(win2, map, 'valid');
sum_map = sum(sum(map_win));

im1 = double(img1);
im2 = double(img2);
[mssim_array ssim_map_array mcs_array cs_map_array] = ssim_index_new(im1, im2, l, K, win);
%new_array = map_win.*ssim_map_array;
mssim = sum(sum(map_win.*ssim_map_array))/sum_map;

mcs = sum(sum(map_win.*cs_map_array))/sum_map;

