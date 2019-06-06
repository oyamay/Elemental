// MUST include this
#include <catch2/catch.hpp>

// File being tested
//#include <El/core/Matrix.hpp>

// Other includes
#include <El.hpp>
#include <hydrogen/meta/TypeTraits.hpp>

TEMPLATE_TEST_CASE(
  "Testing Sequential Matrix","[seq][matrix]",
  float, double)
{
    using value_type = TestType;
    using matrix_type = El::Matrix<value_type, El::Device::CPU>;
    using size_type = typename matrix_type::size_type;

    auto const zero_size = hydrogen::TypeTraits<size_type>::Zero();

    GIVEN("An empty matrix")
    {
        auto mat = matrix_type{};

        THEN ("The matrix has dimension 0x0 with nonzero LDim.")
        {
            CHECK(mat.Height() == zero_size);
            CHECK(mat.Width() == zero_size);
            CHECK(mat.LDim() > zero_size);
            CHECK(mat.MemorySize() == zero_size);
            CHECK(mat.Contiguous());
            CHECK(mat.IsEmpty());
        }

        WHEN ("The matrix is resized")
        {
            mat.Resize(7,11);

            THEN ("The change is reflected in the metadata.")
            {
                CHECK(mat.Height() == size_type{7});
                CHECK(mat.Width() == size_type{11});
                CHECK(mat.LDim() == size_type{7});
                CHECK(mat.MemorySize() == size_type{77});
                CHECK(mat.Contiguous());
                CHECK_FALSE(mat.IsEmpty());
            }
            AND_WHEN ("The matrix is shrunk")
            {
                mat.Resize(5,12);

                THEN ("The change is reflected in the metadata.")
                {
                    CHECK(mat.Height() == size_type{5});
                    CHECK(mat.Width() == size_type{12});
                    CHECK(mat.LDim() == size_type{5});
                    CHECK(mat.MemorySize() == size_type{77});
                    CHECK(mat.Contiguous());
                    CHECK_FALSE(mat.IsEmpty());
                }
            }
            AND_WHEN ("The matrix is expanded")
            {
                mat.Resize(11,13);
                THEN ("The change is reflected in the metadata.")
                {
                    CHECK(mat.Height() == size_type{11});
                    CHECK(mat.Width() == size_type{13});
                    CHECK(mat.LDim() == size_type{11});
                    CHECK(mat.MemorySize() == size_type{11*13});
                    CHECK(mat.Contiguous());
                    CHECK_FALSE(mat.IsEmpty());
                }
            }
            AND_WHEN ("The matrix is resized with a large leading dimension")
            {
                mat.Resize(7,13,11);
                THEN ("The change is reflected in the metadata.")
                {
                    CHECK(mat.Height() == size_type{7});
                    CHECK(mat.Width() == size_type{13});
                    CHECK(mat.LDim() == size_type{11});
                    CHECK(mat.MemorySize() == size_type{11*13});
                    CHECK_FALSE(mat.Contiguous());
                    CHECK_FALSE(mat.IsEmpty());
                }
            }
        }
        WHEN ("The matrix is resized with an invalid leading dimension")
        {
            THEN ("The resize throws and metadata is unchanged.")
            {
                CHECK_THROWS(mat.Resize(7,11,2));
                CHECK(mat.Height() == zero_size);
                CHECK(mat.Width() == zero_size);
                CHECK(mat.LDim() == size_type{1});
                CHECK(mat.MemorySize() == zero_size);
                CHECK(mat.Contiguous());
                CHECK(mat.IsEmpty());
            }
        }
    }

    GIVEN ("A nontrivial matrix")
    {
        auto mat = matrix_type{7,11};
        mat(0,0) = value_type(0.0);

        WHEN ("A submatrix is viewed")
        {
            auto view = El::View(mat, El::IR(0,2), El::IR(0,3));
            THEN ("It can be resized with the same dimensions")
            {
                REQUIRE_NOTHROW(view.Resize(view.Height(), view.Width()));
                CHECK(view.Viewing());
                CHECK(view.FixedSize());
                CHECK(view.Height() == 2);
                CHECK(view.Width() == 3);
                // This borders on checking the View function, but eh
                CHECK(view.LDim() == mat.LDim());
                CHECK(view.MemorySize() == zero_size);
            }
            AND_WHEN ("A value is updated in the matrix")
            {
                mat(0,0) = value_type(1.23);
                THEN ("The update is reflected in the view.")
                {
                    CHECK(view(0,0) == value_type(1.23));
                }
            }
            AND_WHEN ("A value is updated in the view")
            {
                view(0,0) = value_type(3.21);
                THEN ("The update is reflected in the original.")
                {
                    CHECK(mat(0,0) == value_type(3.21));
                }
            }
        }
    }
}
